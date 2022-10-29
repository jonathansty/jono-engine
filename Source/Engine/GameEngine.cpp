#include "engine.pch.h"

#include "AbstractGame.h"
#include "ContactListener.h"
#include "GameEngine.h"

#include "debug_overlays/ImGuiOverlays.h"
#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/RTTIDebugOverlay.h"

#include "core/ResourceLoader.h"

#include "AudioSystem.h"
#include "Font.h"
#include "Graphics/Graphics.h"
#include "InputManager.h"
#include "PrecisionTimer.h"

#include "Core/Logging.h"
#include "Engine/Core/Material.h"
#include "Engine/Core/MaterialResource.h"
#include "Engine/Core/TextureResource.h"

#include "Graphics/ShaderCompiler.h"

#include "CommonStates.h"
#include "Effects.h"
#include "Graphics/Perf.h"
#include "Graphics/ShaderCache.h"
#include "Memory.h"

#include <SDL2/SDL.h>

int g_DebugMode = 0;

static constexpr uint32_t max_task_threads = 8;

// Static task scheduler used for loading assets and executing multi threaded work loads
enki::TaskScheduler* GameEngine::s_TaskScheduler;

// Thread ID used to identify if we are on the main thread.
std::thread::id GameEngine::s_main_thread;

// #TODO: Remove this once full 2D graphics has been refactored into it's own context
#if FEATURE_D2D
using Graphics::bitmap_interpolation_mode;
#endif

GameEngine::GameEngine()
		: m_hInstance(0)
		, m_hWindow(NULL)
		, m_Icon(0)
		, m_SmallIcon(0) //changed in june 2014, reset to false in dec 2014
		, m_WindowWidth(0)
		, m_WindowHeight(0)
		, m_ShouldSleep(true)
		, m_Game(nullptr)
		, m_CanPaint2D(false)
		, m_VSyncEnabled(true)
		, m_DefaultFont(nullptr)
		, m_InputManager(nullptr)
		, m_AudioSystem(nullptr)
		, m_PhysicsStepEnabled(true)
		, m_ViewportIsFocused(true) // TODO: When implementing some kind of editor system this should be updating
		, m_RecreateGameTextureRequested(false)
		, m_RecreateSwapchainRequested(false)
		, m_DebugPhysicsRendering(false)
		, m_Gravity(float2(0, 9.81))
		, _show_debuglog(true)
		, _show_viewport(true)
		, _show_imgui_demo(false)
		, _show_implot_demo(false)
		, _show_entity_editor(false)
		, m_Renderer(nullptr)
{
	ASSERT(!GetGlobalContext()->m_Engine);
	GetGlobalContext()->m_Engine = this;

	// Seed the random number generator
	srand((unsigned int)(GetTickCount64()));
}

GameEngine::~GameEngine()
{
}

void GameEngine::set_game(unique_ptr<AbstractGame>&& gamePtr)
{
	m_Game = std::move(gamePtr);
}

void GameEngine::set_title(const string& titleRef)
{
	m_Title = titleRef;
}

int GameEngine::Run(HINSTANCE hInstance, int iCmdShow)
{
	JONO_THREAD("MainThread");

	// Create the IO first as our logging depends on creating the right folder
	m_PlatformIO = IO::create();
	IO::set(m_PlatformIO);
	GetGlobalContext()->m_PlatformIO = m_PlatformIO.get();

	// Create all the singletons needed by the game, the game engine singleton is initialized from run_game
	Logger::create();
	ResourceLoader::create();

	// Then we initialize the logger as this might create a log file
	Logger::instance()->init();


	// Now we can start logging information and we mount our resources volume.
	LOG_INFO(IO, "Mounting resources directory.");
	m_PlatformIO->mount("Resources");

	ASSERTMSG(m_Game, "No game has been setup! Make sure to first create a game instance before launching the engine!");
	if (m_Game)
	{
		MEMORY_TAG(MemoryCategory::Game);
		m_Game->configure_engine(this->m_EngineCfg);
	}

	// Validate engine settings
	ASSERTMSG(!(m_EngineCfg.d2d_use && (m_EngineCfg.d3d_use && m_EngineCfg.d3d_msaa_mode != MSAAMode::Off)), " Currently the engine does not support rendering D2D with MSAA because DrawText does not respond correctly!");

	s_main_thread = std::this_thread::get_id();

	// initialize d2d for WIC
	SUCCEEDED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));

	// Setup our default overlays
	m_OverlayManager = std::make_shared<OverlayManager>();
	m_MetricsOverlay = new MetricsOverlay(true);

	m_OverlayManager->register_overlay(m_MetricsOverlay);
	m_OverlayManager->register_overlay(new RTTIDebugOverlay());
	m_OverlayManager->register_overlay(new ImGuiDemoOverlay());
	m_OverlayManager->register_overlay(new ImGuiAboutOverlay());

	// set the instance member variable of the game engine
	this->m_hInstance = hInstance;

	// Initialize enkiTS
	ASSERT(!GetGlobalContext()->m_TaskScheduler);
	GetGlobalContext()->m_TaskScheduler = Tasks::get_scheduler();

	GetGlobalContext()->m_TaskScheduler->Initialize();

	struct InitTask : enki::IPinnedTask
	{
		InitTask(uint32_t threadNum)
				: IPinnedTask(threadNum) {}

		void Execute() override
		{
			LOG_INFO(System, "Initializing Task thread {}.", threadNum - 1);

			std::wstring name = fmt::format(L"TaskThread {}", threadNum - 1);
			::SetThreadDescription(GetCurrentThread(), name.c_str());
			SUCCEEDED(::CoInitialize(NULL));
		}
	};

	::SetThreadDescription(GetCurrentThread(), L"MainThread");

	enki::TaskScheduler* taskScheduler = GetGlobalContext()->m_TaskScheduler;
	std::vector<std::unique_ptr<InitTask>> tasks;
	for (uint32_t i = 1; i < taskScheduler->GetNumTaskThreads(); ++i)
	{
		tasks.push_back(std::make_unique<InitTask>(i));
		taskScheduler->AddPinnedTask(tasks.back().get());
	}
	taskScheduler->RunPinnedTasks();
	taskScheduler->WaitforAll();


	//Initialize the high precision timers
	m_FrameTimer = make_unique<PrecisionTimer>();
	m_FrameTimer->reset();

	m_InputManager = make_unique<InputManager>();
	m_InputManager->init();

	ASSERT(!GetGlobalContext()->m_InputManager);
	GetGlobalContext()->m_InputManager = m_InputManager.get();

	// Sound system
#if FEATURE_XAUDIO
	m_AudioSystem = make_unique<XAudioSystem>();
	m_AudioSystem->init();
#endif

#ifdef _DEBUG
	// Log out some test messages to make sure our logging is working
	LOG_VERBOSE(Unknown, "Test verbose message");
	LOG_INFO(Unknown, "Test info message");
	LOG_WARNING(Unknown, "Test warning message");
	LOG_ERROR(Unknown, "Test error message");
#endif



	//_render_thread = std::make_unique<RenderThread>();
	//_render_thread->wait_for_stage(RenderThread::Stage::Running);
	//_render_thread->terminate();
	//_render_thread->wait_for_stage(RenderThread::Stage::Terminated);
	//_render_thread->join();

	m_Renderer = std::make_shared<Graphics::Renderer>();
	m_Renderer->Init(m_EngineCfg, m_GameCfg, m_CommandLine);

	// Game Initialization
	if (m_Game)
	{
		MEMORY_TAG(MemoryCategory::Game);
		m_Game->initialize(m_GameCfg);
	}
	apply_settings(m_GameCfg);

	SDL_Init(SDL_INIT_EVERYTHING);

	if (!this->InitWindow(iCmdShow))
	{
		MessageBoxA(NULL, "Open window failed", "error", MB_OK);
		return false;
	}

	m_Renderer->InitForWindow(m_Window);
	Graphics::init(m_Renderer->get_ctx());

	m_ViewportWidth = m_Renderer->GetDrawableWidth();
	m_ViewportHeight = m_Renderer->GetDrawableHeight();
	m_ViewportPos = { 0.0f, 0.0f };

	m_DefaultFont = make_shared<Font>("Consolas", 12.0f);

	TextureHandle::init_default();

	LOG_INFO(System, "Initialising worlds...");
	{
		m_World = std::make_shared<framework::World>();
		m_World->init();

		m_RenderWorld = std::make_shared<RenderWorld>();
		m_RenderWorld->init();
	}
	LOG_INFO(System, "Finished initialising worlds.");


	{
		ImGui::SetAllocatorFunctions(
				[](size_t size, void*)
				{
					return std::malloc(size);
				},
				[](void* mem, void*)
				{
					return std::free(mem);
				});
		ImGui::CreateContext();
		ImPlot::CreateContext();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		// ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
		// ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

		ImGui::GetIO().FontDefault = nullptr;
		ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto-Regular.ttf", 16.0f);

		// Set colors
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.33f, 0.33f, 0.33f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.98f, 0.57f, 0.26f, 0.40f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.55f, 0.26f, 0.67f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.95f, 0.56f, 0.06f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.58f, 0.26f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.56f, 0.24f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.67f, 0.26f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.98f, 0.55f, 0.26f, 0.40f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.60f, 0.26f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.87f, 0.59f, 0.05f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(1.00f, 0.60f, 0.00f, 0.65f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.62f, 0.26f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.67f, 0.26f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.37f, 0.10f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.27f, 0.10f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.57f, 0.26f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.55f, 0.26f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.60f, 0.26f, 0.95f);
			colors[ImGuiCol_Tab] = ImVec4(0.83f, 0.53f, 0.14f, 0.70f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.99f, 0.78f, 0.58f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.56f, 0.00f, 0.86f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.34f, 0.27f, 0.21f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.88f, 0.47f, 0.10f, 0.45f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.98f, 0.62f, 0.26f, 0.70f);
			colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.98f, 0.62f, 0.26f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.65f, 0.26f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		}

		Graphics::DeviceContext ctx = m_Renderer->get_ctx();
		ImGui_ImplSDL2_InitForD3D(m_Window);
		ImGui_ImplDX11_Init(ctx._device.Get(), ctx._ctx.Get());

	}

#pragma region Box2D
	// Initialize Box2D
	// Define the gravity vector.
	b2Vec2 gravity((float)m_Gravity.x, (float)m_Gravity.y);

	// Construct a world object, which will hold and simulate the rigid bodies.
	m_Box2DWorld = make_shared<b2World>(gravity);
	m_Box2DContactFilter = make_shared<b2ContactFilter>();

	m_Box2DWorld->SetContactFilter(m_Box2DContactFilter.get());
	m_Box2DWorld->SetContactListener(this);

#if FEATURE_D2D
	m_Box2DDebugRenderer.SetFlags(b2Draw::e_shapeBit);
	m_Box2DDebugRenderer.AppendFlags(b2Draw::e_centerOfMassBit);
	m_Box2DDebugRenderer.AppendFlags(b2Draw::e_jointBit);
	m_Box2DDebugRenderer.AppendFlags(b2Draw::e_pairBit);
	m_Box2DWorld->SetDebugDraw(&m_Box2DDebugRenderer);
#endif
#pragma endregion

	// User defined functions for start of the game
	if (m_Game)
	{
		MEMORY_TAG(MemoryCategory::Game);
		m_Game->start();
	}

	// Ensure world has default setup
	if (!get_render_world()->get_view_camera())
	{
		RenderWorldCameraRef camera = get_render_world()->create_camera();
		ImVec2 size = GameEngine::instance()->get_viewport_size();
		const float aspect = (float)size.x / (float)size.y;
		const float near_plane = 5.0f;
		const float far_plane = 1000.0f;

		// Create Cam 1
		RenderWorldCamera::CameraSettings settings{};
		settings.aspect = aspect;
		settings.far_clip = far_plane;
		settings.near_clip = near_plane;
		settings.fov = hlslpp::radians(float1(60.0f));
		settings.projection_type = RenderWorldCamera::Projection::Perspective;
		camera->set_settings(settings);
		camera->set_view(float4x4::translation({ 0.0f, 1.5f, -25.0f }));
	}



	// Initialize our performance tracking
	Perf::initialize(m_Renderer->get_raw_device());

	// Initialize our GPU timers
	for (u32 j = 0; j < std::size(m_GpuTimings); ++j)
	{
		m_GpuTimings[j] = Perf::Timer(m_Renderer->get_raw_device());
	}

	// Timer to track the elapsed time in the game
	PrecisionTimer full_frame_timer{};
	f64 time_elapsed = 0.0;
	f64 time_previous = 0.0;
	f64 time_lag = 0.0; 

	m_IsRunning = m_Game ? true : false;
	while (m_IsRunning)
	{
		JONO_FRAME("Frame");

		full_frame_timer.start();

		{
			JONO_EVENT("PollInput");

			Timer t{};
			t.Start();
			// Process all window messages

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				GameEngine::instance()->ProcessEvent(e);
			}

			MSG msg{};
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			t.Stop();
			m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::EventHandlingCPU, t.GetTimeInMS());
		}

		// Running might have been updated by the windows message loop. Handle this here.
		if (!m_IsRunning)
		{
			break;
		}

		{


			// Execute the game simulation loop
			{
				JONO_EVENT("Simulation");
				f64 current = time_elapsed;
				f64 elapsed = current - time_previous; 
				m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::FrameTime, (float)(elapsed * 1000.0f));

				time_previous = current; 
				time_lag += elapsed;

				Timer t{};
				t.Start();
				while (time_lag >= c_FixedPhysicsTimestep)
				{
					// Call the Game Tick method
					if (m_Game)
					{
						MEMORY_TAG(MemoryCategory::Game);
						m_Game->tick(c_FixedPhysicsTimestep);
					}

					int32 velocityIterations = 6;
					int32 positionIterations = 2;
					if (m_PhysicsStepEnabled)
					{
						m_Box2DWorld->Step((float)c_FixedPhysicsTimestep, velocityIterations, positionIterations);
					}

					// Step generates contact lists, pass to Listeners and clear the vector
					CallListeners();
					time_lag -= c_FixedPhysicsTimestep;

					// Input manager update takes care of swapping the state
					m_InputManager->update();
				}
				t.Stop();
				m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::GameUpdateCPU, (float)t.GetTimeInMS());
			}


			Perf::begin_frame(m_Renderer->get_raw_device_context());
			if (m_RecreateSwapchainRequested)
			{
				m_Renderer->resize_swapchain(m_WindowWidth, m_WindowHeight);
				m_RecreateSwapchainRequested = false;
			}

			// Recreating the game viewport texture needs to happen before running IMGUI and the actual rendering
			{
				JONO_EVENT("DebugUI");
				ImGui_ImplDX11_NewFrame();

				//ImVec2 displaySize = { (float)m_Renderer->GetDrawableWidth(), (float)m_Renderer->GetDrawableHeight() };
				ImGui_ImplSDL2_NewFrame(nullptr);

				ImGui::NewFrame();

				build_ui();
				ImVec2 game_width = { get_width() / 2.0f, get_height() / 2.0f };
				ImGui::SetNextWindowSize(game_width, ImGuiCond_FirstUseEver);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
				ImGui::PopStyleVar(1);

				if (m_Game)
				{
					MEMORY_TAG(MemoryCategory::Debug);
					m_Game->debug_ui();
				}
				m_OverlayManager->render_overlay();

				ImGui::EndFrame();
				ImGui::UpdatePlatformWindows();
				ImGui::Render();
			}

		}

		ResourceLoader::instance()->update();

		// Process the previous frame gpu timers here to allow our update thread to run first
		if (Perf::can_collect())
		{
			JONO_EVENT("PerfCollect");
			size_t current = Perf::get_previous_frame_resource_index();

			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestampDisjoint;
			UINT64 start;
			UINT64 end;

			if (Perf::collect_disjoint(m_Renderer->get_raw_device_context(), timestampDisjoint))
			{
				auto& timing_data = m_GpuTimings[current];
				f64 cpuTime;
				timing_data.flush(m_Renderer->get_raw_device_context(), start, end, cpuTime);

				double diff = (double)(end - start) / (double)timestampDisjoint.Frequency;
				m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::RenderGPU, (float)(diff * 1000.0));
				m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::RenderCPU, (float)(cpuTime * 1000.0));
			}
		}


		render();

		PrecisionTimer present_timer{};
		present_timer.reset();
		present_timer.start();
		present();
		present_timer.stop();


		// Update CPU only timings
		{
			JONO_EVENT("FrameLimiter");
			m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::PresentCPU, present_timer.get_delta_time() * 1000.0);
			if (m_EngineCfg.max_frame_time > 0.0)
			{
				f64 targetTimeMs = m_EngineCfg.max_frame_time;

				// Get the current frame time
				f64 framet = full_frame_timer.get_delta_time();
				f64 time_to_sleep = targetTimeMs - framet;

				Perf::precise_sleep(time_to_sleep);
			}
		}
		full_frame_timer.stop();
		f64 framet = full_frame_timer.get_delta_time();
		time_elapsed += framet;

	}


	// User defined code for exiting the game
	if (m_Game)
	{
		MEMORY_TAG(MemoryCategory::Game);
		m_Game->end();
	}
	m_Game.reset();

	// Make sure all tasks have finished before shutting down
	Tasks::get_scheduler()->WaitforAllAndShutdown();

	Perf::shutdown();

	ResourceLoader::instance()->unload_all();
	ResourceLoader::shutdown();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();

	ImGui::DestroyContext();

	SDL_DestroyWindow(m_Window);
	SDL_Quit();

	// Teardown graphics resources and windows procedures
	{
		m_DefaultFont.reset();

		Graphics::deinit();
		m_Renderer->DeInit();
		m_Renderer = nullptr;

		::CoUninitialize();
	}

	return 0;
}

#if FEATURE_D2D
void GameEngine::d2d_render()
{
	// Plan to modernize the 2D and deprecate Direct2D
	// 1. Collect all the draw commands in buffers and capture the required data
	// 2. during end_paint 'flush' draw commands and create required vertex buffers
	// 3. Execute each draw command binding the right buffers and views
	if (!m_D2DRenderContext)
	{
		m_D2DRenderContext = std::make_unique<Graphics::D2DRenderContext>(m_Renderer.get(), m_Renderer->get_raw_d2d_factory(), m_Renderer->get_2d_draw_ctx(), m_Renderer->get_2d_color_brush(), m_DefaultFont.get());
	}
	Graphics::D2DRenderContext& context = *m_D2DRenderContext;
	context.begin_paint(m_Renderer.get(), m_Renderer->get_raw_d2d_factory(), m_Renderer->get_2d_draw_ctx(), m_Renderer->get_2d_color_brush(), m_DefaultFont.get());

	auto size = this->get_viewport_size();

	m_CanPaint2D = true;
	// make sure tvp.Heighthe view matrix is taken in account
	context.set_world_matrix(float4x4::identity());

	{
		GPU_SCOPED_EVENT(m_Renderer->get_raw_annotation(), "D2D:Paint");
		if (m_Game)
		{
			MEMORY_TAG(MemoryCategory::Game);
			m_Game->paint(context);
		}
	}

	// draw Box2D debug rendering
	// http://www.iforce2d.net/b2dtut/debug-draw
	if (m_DebugPhysicsRendering)
	{
		// dimming rect in screenspace
		context.set_world_matrix(float4x4::identity());
		float4x4 matView = context.get_view_matrix();
		context.set_view_matrix(float4x4::identity());
		context.set_color(MK_COLOR(0, 0, 0, 127));
		context.fill_rect(0, 0, get_width(), get_height());
		context.set_view_matrix(matView);

		m_Box2DDebugRenderer.set_draw_ctx(&context);
		m_Box2DWorld->DebugDraw();
		m_Box2DDebugRenderer.set_draw_ctx(nullptr);
	}

	m_CanPaint2D = false;
	bool result = context.end_paint();

	// if drawing failed, terminate the game
	if (!result)
	{
		PostMessage(GameEngine::get_window(), WM_DESTROY, 0, 0);
	}
}
#endif

bool GameEngine::InitWindow(int iCmdShow)
{
	// Calculate the window size and position based upon the game size
	int iWindowWidth = m_WindowWidth;
	int iWindowHeight = m_WindowHeight;
	int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2;
	int iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;

	u32 flags = 0;
	if (m_GameCfg.m_WindowFlags & GameCfg::WindowFlags::StartMaximized)
	{
		flags |= SDL_WINDOW_MAXIMIZED;
	}

	std::wstring title = std::wstring(m_Title.begin(), m_Title.end());
	m_Window = SDL_CreateWindow(m_Title.c_str(), iXWindowPos, iYWindowPos, iWindowWidth, iWindowHeight, flags);
	if (!m_Window)
	{
		FAILMSG("Failed to create the SDL window.");
		return false;
	}

	SDL_GetWindowSize(m_Window, &m_WindowWidth, &m_WindowHeight);
	assert(m_WindowWidth > 0 && m_WindowHeight > 0);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(m_Window, &info))
	{
		m_hWindow = info.info.win.window;
		m_hInstance = info.info.win.hinstance;
	}


	return true;
}

void GameEngine::Quit()
{
	this->m_IsRunning = false;
}

void GameEngine::enable_aa(bool isEnabled)
{
#if 0
	_d2d_aa_mode = isEnabled ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
#if FEATURE_D2D
	if (_d2d_rt)
	{
		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
	}
#endif
	#endif
}

void GameEngine::enable_physics_debug_rendering(bool isEnabled)
{
	m_DebugPhysicsRendering = isEnabled;
}

HINSTANCE GameEngine::get_instance() const
{
	return m_hInstance;
}

HWND GameEngine::get_window() const
{
	return m_hWindow;
}

string GameEngine::get_title() const
{
	return m_Title;
}

WORD GameEngine::get_icon() const
{
	return m_Icon;
}

WORD GameEngine::get_small_icon() const
{
	return m_SmallIcon;
}

ImVec2 GameEngine::get_window_size() const
{
	return { (float)m_WindowWidth, (float)m_WindowHeight };
}

ImVec2 GameEngine::get_viewport_size(int) const
{
	return { (float)m_ViewportWidth, (float)m_ViewportHeight };
}

ImVec2 GameEngine::get_viewport_pos(int id /*= 0*/) const
{
	RECT rect;
	GetWindowRect(get_window(), &rect);

	return {
		(float)m_ViewportPos.x - rect.left, (float)m_ViewportPos.y - rect.top
	};
}

int GameEngine::get_width() const
{
	return m_WindowWidth;
}

int GameEngine::get_height() const
{
	return m_WindowHeight;
}

bool GameEngine::get_sleep() const
{
	return m_ShouldSleep ? true : false;
}

float2 GameEngine::get_mouse_pos_in_window() const
{
	RECT rect;
	if (GetWindowRect(get_window(), &rect))
	{
		return float2{ (float)m_InputManager->get_mouse_position().x, (float)m_InputManager->get_mouse_position().y } - float2(rect.left, rect.top);
	}
	return {};
}

float2 GameEngine::get_mouse_pos_in_viewport() const
{
	float2 tmp = float2(m_InputManager->get_mouse_position());
	return float2{ (float)tmp.x, (float)tmp.y } - m_ViewportPos;
}

unique_ptr<XAudioSystem> const& GameEngine::get_audio_system() const
{
	return m_AudioSystem;
}

void GameEngine::set_icon(WORD wIcon)
{
	m_Icon = wIcon;
}

void GameEngine::set_small_icon(WORD wSmallIcon)
{
	m_SmallIcon = wSmallIcon;
}

void GameEngine::set_width(int iWidth)
{
	m_WindowWidth = iWidth;
	assert(m_WindowWidth > 0);
}

void GameEngine::set_height(int iHeight)
{
	m_WindowHeight = iHeight;
	assert(m_WindowHeight > 0);
}

void GameEngine::set_physics_step(bool bEnabled)
{
	m_PhysicsStepEnabled = bEnabled;
}

bool GameEngine::is_viewport_focused() const
{
	return m_ViewportIsFocused;
}

bool GameEngine::is_input_captured() const
{
	if (is_viewport_focused())
	{
		return false;
	}

	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;

}

void GameEngine::set_sleep(bool bSleep)
{
	if (m_FrameTimer == nullptr)
		return;

	m_ShouldSleep = bSleep;
	if (bSleep)
	{
		m_FrameTimer->stop();
	}
	else
	{
		m_FrameTimer->start();
	}
}

void GameEngine::enable_vsync(bool bEnable)
{
	m_VSyncEnabled = bEnable;
}

void GameEngine::apply_settings(GameCfg& game_settings)
{
	enable_aa(m_EngineCfg.d2d_use_aa);

	set_width(game_settings.m_WindowWidth);
	set_height(game_settings.m_WindowHeight);
	set_title(game_settings.m_WindowTitle);
	enable_vsync(game_settings.m_WindowFlags & GameCfg::WindowFlags::EnableVSync);
}

void GameEngine::set_vsync(bool vsync)
{
	m_VSyncEnabled = vsync;
}

bool GameEngine::get_vsync()
{
	return m_VSyncEnabled;
}

std::shared_ptr<OverlayManager> const& GameEngine::get_overlay_manager() const
{
	return m_OverlayManager;
}

// Input methods
bool GameEngine::is_key_down(int key) const
{
	return m_InputManager->is_key_down((KeyCode)key);
}

bool GameEngine::is_key_pressed(int key) const
{
	return m_InputManager->is_key_pressed((KeyCode)key);
}

bool GameEngine::is_key_released(int key) const
{
	return m_InputManager->is_key_released((KeyCode)key);
}

bool GameEngine::is_mouse_button_down(int button) const
{
	return m_InputManager->is_mouse_button_down(button);
}

bool GameEngine::is_mouse_button_pressed(int button) const
{
	return m_InputManager->is_mouse_button_pressed(button);
}

bool GameEngine::is_mouse_button_released(int button) const
{
	return m_InputManager->is_mouse_button_released(button);
}

void GameEngine::ProcessEvent(SDL_Event& e)
{
	// Get window rectangle and HDC
	RECT windowClientRect;
	GetClientRect(m_hWindow, &windowClientRect);

	RECT usedClientRect;
	usedClientRect.left = 0;
	usedClientRect.top = 0;
	usedClientRect.right = get_width();
	usedClientRect.bottom = get_height();

	// Route Windows messages to game engine member functions
	switch (e.type)
	{
		case SDL_WINDOWEVENT:
		{
			if (e.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				GameEngine::instance()->Quit();
				return;
			}
			else if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED)
			{
				SetWindowPos(m_hWindow, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
				RECT r;
				::GetClientRect(m_hWindow, &r);
				m_WindowWidth = r.right - r.left;
				m_WindowHeight = r.bottom - r.top;

				this->m_RecreateSwapchainRequested = true;
			}
			else if (e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				RECT r;
				::GetClientRect(m_hWindow, &r);
				m_WindowWidth = r.right - r.left;
				m_WindowHeight = r.bottom - r.top;

				this->m_RecreateSwapchainRequested = true;
				return;
			}
		}
		break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if ((e.key.keysym.mod & KMOD_CTRL ) && e.key.keysym.sym == SDLK_F9)
			{
				m_OverlayManager->set_visible(!m_OverlayManager->get_visible());
			}
			break;
	}

	bool handled = false;

	if (ImGui::GetCurrentContext() != nullptr)
	{
		 bool bWantImGuiCapture = ImGui::GetIO().WantCaptureKeyboard ||
								 ImGui::GetIO().WantCaptureMouse;

		if (bWantImGuiCapture)
		{
			ImGui_ImplSDL2_ProcessEvent(&e);
		}
	}

	if (!handled)
	{
		// Input manager doesn't consume the inputs
		handled |= m_InputManager->handle_events(e);
	}

	if (handled)
	{
		return;
	}
}


// Box2D overloads
void GameEngine::BeginContact(b2Contact* contactPtr)
{
	b2Fixture* fixAPtr = contactPtr->GetFixtureA();
	b2Fixture* fixBPtr = contactPtr->GetFixtureB();

	ContactData contactData;
	// fixture userdata is ActorPtr
	// body UserData is ContactlistenerPtr to call

	//is A a contactlistener?
	if (fixAPtr->GetBody()->GetUserData() != nullptr)
	{
		// the object to call
		contactData.contactListenerPtr = fixAPtr->GetBody()->GetUserData();
		// the actor of this contactlistener
		contactData.actThisPtr = fixAPtr->GetUserData();
		// the other actor that made contact
		contactData.actOtherPtr = fixBPtr->GetUserData();
		// check for removed actors, this method can be called from within the PhysicsActor destructor
		// when one of two overlapping actors is deleted
		if (contactData.actThisPtr != nullptr && contactData.actOtherPtr != nullptr)
		{
			// store in caller list
			m_Box2DBeginContactData.push_back(contactData);
		}
	}

	//is B a contactlistener?
	if (fixBPtr->GetBody()->GetUserData() != nullptr)
	{
		// the object to call
		contactData.contactListenerPtr = fixBPtr->GetBody()->GetUserData();
		// the actor of this contactlistener
		contactData.actThisPtr = fixBPtr->GetUserData();
		// the other actor that made contact
		contactData.actOtherPtr = fixAPtr->GetUserData();
		// check for removed actors, this method can be called from within the PhysicsActor destructor
		// when one of two overlapping actors is deleted
		if (contactData.actThisPtr != nullptr && contactData.actOtherPtr != nullptr)
		{
			// store in caller list
			m_Box2DBeginContactData.push_back(contactData);
		}
	}
};

void GameEngine::EndContact(b2Contact* contactPtr)
{
	b2Fixture* fixAPtr = contactPtr->GetFixtureA();
	b2Fixture* fixBPtr = contactPtr->GetFixtureB();

	ContactData contactData;
	// fixture userdata is ActorPtr
	// body UserData is ContactlistenerPtr to call

	//is A a contactlistener?
	if (fixAPtr->GetBody()->GetUserData() != nullptr)
	{
		// the object to call
		contactData.contactListenerPtr = fixAPtr->GetBody()->GetUserData();
		// the actor of this contactlistener
		contactData.actThisPtr = fixAPtr->GetUserData();
		// the other actor that made contact
		contactData.actOtherPtr = fixBPtr->GetUserData();
		// check for removed actors, this method can be called from within the PhysicsActor destructor
		// when one of two overlapping actors is deleted
		if (contactData.actThisPtr != nullptr && contactData.actOtherPtr != nullptr)
		{
			// store in caller list
			m_Box2DEndContactData.push_back(contactData);
		}
	}

	//is B a contactlistener?
	if (fixBPtr->GetBody()->GetUserData() != nullptr)
	{
		// the object to call
		contactData.contactListenerPtr = fixBPtr->GetBody()->GetUserData();
		// the actor of this contactlistener
		contactData.actThisPtr = fixBPtr->GetUserData();
		// the other actor that made contact
		contactData.actOtherPtr = fixAPtr->GetUserData();
		// check for removed actors, this method can be called from within the PhysicsActor destructor
		// when one of two overlapping actors is deleted
		if (contactData.actThisPtr != nullptr && contactData.actOtherPtr != nullptr)
		{
			// store in caller list
			m_Box2DEndContactData.push_back(contactData);
		}
	}
};

void GameEngine::PreSolve(b2Contact*, const b2Manifold*)
{
}

void GameEngine::PostSolve(b2Contact* contactPtr, const b2ContactImpulse* impulsePtr)
{
	b2Fixture* fixAPtr = contactPtr->GetFixtureA();
	b2Fixture* fixBPtr = contactPtr->GetFixtureB();

	ImpulseData impulseData;
	impulseData.contactListenerAPtr = fixAPtr->GetBody()->GetUserData();
	impulseData.contactListenerBPtr = fixBPtr->GetBody()->GetUserData();
	impulseData.actAPtr = fixAPtr->GetUserData();
	impulseData.actBPtr = fixBPtr->GetUserData();

	// normalImpulses[1] seems to be always 0, add them up
	if (impulsePtr->count > 0)
		impulseData.impulseA = impulsePtr->normalImpulses[0];
	if (impulsePtr->count > 1)
		impulseData.impulseB = impulsePtr->normalImpulses[1];

	double sum = impulseData.impulseA + impulseData.impulseB;
	impulseData.impulseA = impulseData.impulseB = sum;

	if (sum > 0.00001)
		m_Box2DImpulseData.push_back(impulseData);
}

void GameEngine::CallListeners()
{
	// begin contact
	for (size_t i = 0; i < m_Box2DBeginContactData.size(); i++)
	{
		ContactListener* contactListenerPtr = reinterpret_cast<ContactListener*>(m_Box2DBeginContactData[i].contactListenerPtr);
		contactListenerPtr->BeginContact(
				reinterpret_cast<PhysicsActor*>(m_Box2DBeginContactData[i].actThisPtr),
				reinterpret_cast<PhysicsActor*>(m_Box2DBeginContactData[i].actOtherPtr));
	}
	m_Box2DBeginContactData.clear();

	// end contact
	for (size_t i = 0; i < m_Box2DEndContactData.size(); i++)
	{
		ContactListener* contactListenerPtr = reinterpret_cast<ContactListener*>(m_Box2DEndContactData[i].contactListenerPtr);
		contactListenerPtr->EndContact(
				reinterpret_cast<PhysicsActor*>(m_Box2DEndContactData[i].actThisPtr),
				reinterpret_cast<PhysicsActor*>(m_Box2DEndContactData[i].actOtherPtr));
	}
	m_Box2DEndContactData.clear();

	// impulses
	for (size_t i = 0; i < m_Box2DImpulseData.size(); i++)
	{
		ContactListener* contactListenerAPtr = reinterpret_cast<ContactListener*>(m_Box2DImpulseData[i].contactListenerAPtr);
		ContactListener* contactListenerBPtr = reinterpret_cast<ContactListener*>(m_Box2DImpulseData[i].contactListenerBPtr);
		if (contactListenerAPtr != nullptr)
			contactListenerAPtr->ContactImpulse(reinterpret_cast<PhysicsActor*>(m_Box2DImpulseData[i].actAPtr), m_Box2DImpulseData[i].impulseA);
		if (contactListenerBPtr != nullptr)
			contactListenerBPtr->ContactImpulse(reinterpret_cast<PhysicsActor*>(m_Box2DImpulseData[i].actBPtr), m_Box2DImpulseData[i].impulseB);
	}
	m_Box2DImpulseData.clear();
}

// String helpers for hlsl types
template <>
struct fmt::formatter<float4x4> : formatter<string_view>
{
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
	{
		auto it = ctx.begin();
		++it;
		return it;
	}

	template <typename FormatContext>
	constexpr auto format(hlslpp::float4x4 const& m, FormatContext& ctx)
	{
		return fmt::format_to(ctx.out(),
				"[{:.1f} {:.1f} {:.1f} {:.1f}, {:.1f} {:.1f} {:.1f} {:.1f}, {:.1f} {:.1f} {:.1f} {:.1f}, {:.1f} {:.1f} {:.1f} {:.1f}]",
				m.f32_128_0[0],
				m.f32_128_0[1],
				m.f32_128_0[2],
				m.f32_128_0[3],
				m.f32_128_1[0],
				m.f32_128_1[1],
				m.f32_128_1[2],
				m.f32_128_1[3],
				m.f32_128_2[0],
				m.f32_128_2[1],
				m.f32_128_2[2],
				m.f32_128_2[3],
				m.f32_128_3[0],
				m.f32_128_3[1],
				m.f32_128_3[2],
				m.f32_128_3[3]);
	}
};

void GameEngine::render_view(Graphics::RenderPass::Value pass)
{
	m_Renderer->render_view(m_RenderWorld, pass);
}

void GameEngine::build_ui()
{
	MEMORY_TAG(MemoryCategory::Debug);

	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		ImGui::Begin("Engine Editor", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		// DockSpace
		ImGuiID dockspace_id = ImGui::GetID("Dockspace##Main");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

		build_menubar();
		build_debug_log();
		build_viewport();

		ImGui::End();
	}

	if (_show_implot_demo)
	{
		ImPlot::ShowDemoWindow(&_show_implot_demo);
	}
}

void GameEngine::render()
{
	JONO_EVENT();
	auto d3d_annotation = m_Renderer->get_raw_annotation();
	GPU_SCOPED_EVENT(d3d_annotation, "Frame");

	size_t idx = Perf::get_current_frame_resource_index();

	// Begin frame gpu timer
	auto& timer = m_GpuTimings[idx];
	timer.begin(m_Renderer->get_raw_device_context());

	m_Renderer->begin_frame();

	// Render 3D before 2D
	if (m_EngineCfg.d3d_use)
	{
		m_Renderer->pre_render(m_RenderWorld);

		GPU_SCOPED_EVENT(d3d_annotation, "Render");
		// Render the shadows
		if(Graphics::s_EnableShadowRendering)
		{
			m_Renderer->render_shadow_pass(m_RenderWorld);
		}

		{
			GPU_SCOPED_EVENT(d3d_annotation, "Main");
			m_Renderer->render_zprepass(m_RenderWorld);
			m_Renderer->render_opaque_pass(m_RenderWorld);
		}
	}

	// Render Direct2D to the swapchain
	if (m_EngineCfg.d2d_use)
	{
#if FEATURE_D2D
		d2d_render();
#else
		LOG_ERROR(System, "Trying to use D2D but the build isn't compiled with D2D enabled!");
		DebugBreak();
#endif
	}

	m_Renderer->render_post(m_RenderWorld, m_OverlayManager);
	m_Renderer->end_frame();
}

void GameEngine::present()
{
	JONO_EVENT();
	auto d3d_ctx = m_Renderer->get_raw_device_context();
	auto d3d_annotation = m_Renderer->get_raw_annotation();
	auto d3d_swapchain = m_Renderer->get_raw_swapchain();

	size_t idx = Perf::get_current_frame_resource_index();

	// Present, 
	GPU_MARKER(d3d_annotation, L"DrawEnd");
	u32 flags = 0;
	if (!m_VSyncEnabled)
	{
		flags |= DXGI_PRESENT_ALLOW_TEARING;
	}
	d3d_swapchain->Present(m_VSyncEnabled ? 1 : 0, flags);

	auto& timer = m_GpuTimings[idx];
	timer.end(d3d_ctx);
	Perf::end_frame(d3d_ctx);

	// Render all other imgui windows
	ImGui::RenderPlatformWindowsDefault();
}

void GameEngine::build_debug_log()
{
	if (!_show_debuglog)
		return;

	if (ImGui::Begin("Output Log", &_show_debuglog, 0))
	{

		static bool s_scroll_to_bottom = true;
		static bool s_shorten_file = true;
		static char s_filter[256] = {};
		ImGui::InputText("Filter", s_filter, 512);
		ImGui::SameLine();
		ImGui::Checkbox("Scroll To Bottom", &s_scroll_to_bottom);
		ImGui::SameLine();
		if(ImGui::Button("Clear"))
		{
			Logger::instance()->clear();
		}
			

		//ImGui::BeginTable("LogData", 3);
		ImGui::BeginChild("123", ImVec2(0, 0),false, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
		auto const& buffer = Logger::instance()->GetBuffer();
		for (auto it = buffer.begin(); it != buffer.end(); ++it)
		{
			LogEntry const* entry = *it;
			if (s_filter[0] != '\0')
			{
				std::string msg = entry->to_message();
				bool bPassed = (msg.find(s_filter) != std::string::npos);
				if (!bPassed)
					continue;
			}

			switch (entry->_severity)
			{
				case Logger::Severity::Info:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3, 0.3, 1.0, 1.0));
					break;
				case Logger::Severity::Warning:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0.7, 0.1, 1.0));
					break;
				case Logger::Severity::Error:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9, 0.1, 0.1, 1.0));
					break;
				case Logger::Severity::Verbose:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
				default:
					break;
			}

			std::string filename = entry->_file ? entry->_file : "null";
			if (s_shorten_file && entry->_file)
			{
				auto path = std::filesystem::path(filename);
				filename = fmt::format("{}", path.filename().string());
			}
			ImGui::Text("[%s(%d)][%s][%s] %s", filename.c_str(), it->_line, logging::to_string(it->_category), logging::to_string(it->_severity), it->_message.c_str());
			ImGui::PopStyleColor();
		}
		if (Logger::instance()->_hasNewMessages && s_scroll_to_bottom)
		{
			Logger::instance()->_hasNewMessages = false;
			ImGui::SetScrollHereY();
		}
		//ImGui::EndTable();
		ImGui::EndChild();
	}
	ImGui::End();
}

void GameEngine::build_viewport()
{
	if (!_show_viewport)
		return;

	if (ImGui::Begin("Viewport##GameViewport", &_show_viewport))
	{
		ImGui::SetCursorPos(ImVec2(0, 0));

		static ImVec2 s_vp_size = ImGui::GetContentRegionAvail();
		static ImVec2 s_vp_pos = ImGui::GetWindowContentRegionMin();
		ImVec2 current_size = ImGui::GetContentRegionAvail();

		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		ImVec2 window_pos = ImGui::GetWindowPos();

		m_ViewportIsFocused = ImGui::IsWindowHovered();

// Enable this to draw a debug rect around the viewport
#if 0
		{
			auto min = vMin;
			auto max = vMax;
			min.x += ImGui::GetWindowPos().x;
			min.y += ImGui::GetWindowPos().y;
			max.x += ImGui::GetWindowPos().x;
			max.y += ImGui::GetWindowPos().y;

			m_ViewportPos.x = min.x;
			m_ViewportPos.y = min.y;
			ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(255, 255, 0, 255));
		}
#endif

		if (s_vp_size.x != current_size.x || s_vp_size.y != current_size.y || s_vp_pos.x != vMin.x || s_vp_pos.y != vMin.y)
		{
			LOG_VERBOSE(Graphics, "Viewport resize detected! From {}x{} to {}x{}", current_size.x, current_size.y, s_vp_size.x, s_vp_size.y);


			// Update the viewport sizes
			s_vp_size = current_size;
			s_vp_pos = vMin;
			m_ViewportWidth = (u32)s_vp_size.x;
			m_ViewportHeight = (u32)s_vp_size.y;
			m_ViewportPos = float2(s_vp_pos.x, s_vp_pos.y);

			m_Renderer->update_viewport(static_cast<u32>(vMin.x), static_cast<u32>(vMin.y), static_cast<u32>(s_vp_size.x), static_cast<u32>(s_vp_size.y));
		}

		// Draw the actual scene image
		ImVec2 max_uv = {};
		max_uv.x = s_vp_size.x / get_width();
		max_uv.y = s_vp_size.y / get_height();

		m_ViewportPos = float2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		ImGui::Image(m_Renderer->get_raw_output_non_msaa_srv(), s_vp_size, ImVec2(0, 0), max_uv);


		//ImGuizmo::SetDrawlist();
		//ImGuizmo::SetRect(_viewport_pos.x, _viewport_pos.y, float1(_viewport_width), float1(_viewport_height));

		get_overlay_manager()->render_viewport();

// Draw any gizmos
#if 0

		constexpr f32 l = 0.f;
		auto camera = _render_world->get_camera(0);
		float4x4 view_mtx = camera->get_view();
		float4x4 proj_mtx = camera->get_proj();
		float view[16];
		float proj[16];
		hlslpp::store(view_mtx, view);
		hlslpp::store(proj_mtx, proj);

		// #TODO: Render gizmos for the selected entity
		static float4x4 matrix = float4x4::identity();
		ImGuizmo::Manipulate(view, proj, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, (float*)&matrix);
#endif
	}
	ImGui::End();
}

void GameEngine::build_menubar()
{
	if (ImGui::BeginMenuBar())
	{
		if (_build_menu)
		{
			_build_menu(BuildMenuOrder::First);
		}

		if (ImGui::BeginMenu("Simulation"))
		{
			static bool s_should_simulate = true;
			if (ImGui::MenuItem(s_should_simulate ? "Stop" : "Start", "", nullptr))
			{
				s_should_simulate = !s_should_simulate;
				fmt::printf("Toggling Simulation %s.\n", s_should_simulate ? "On" : "Off");

				this->set_sleep(!s_should_simulate);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{
			if (ImGui::MenuItem("[DEMO] ImPlot"))
			{
				_show_implot_demo = !_show_implot_demo;
			}

			if (ImGui::MenuItem("Debug Log"))
			{
				_show_debuglog = !_show_debuglog;
			}
			if (ImGui::MenuItem("Viewport"))
			{
				_show_viewport = !_show_viewport;
			}
			if (ImGui::MenuItem("Entity Editor"))
			{
				_show_entity_editor = !_show_entity_editor;
				if (auto editor = get_overlay_manager()->get_overlay("EntityEditor"))
				{
					get_overlay_manager()->get_overlay("EntityDebugOverlay")->set_visible(_show_entity_editor);
				}
			}

			if (ImGui::BeginMenu("Overlays"))
			{
				if (ImGui::IsItemClicked())
				{
					get_overlay_manager()->set_visible(!get_overlay_manager()->get_visible());
				}

				for (auto overlay : get_overlay_manager()->get_overlays())
				{
					bool enabled = overlay->get_visible();
					if (ImGui::Checkbox(fmt::format("##{}", overlay->get_name()).c_str(), &enabled))
					{
						overlay->set_visible(enabled);
					}

					ImGui::SameLine();
					if (ImGui::MenuItem(overlay->get_name()))
					{
						overlay->set_visible(!overlay->get_visible());
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (_build_menu)
		{
			_build_menu(BuildMenuOrder::Last);
		}

		ImGui::EndMenuBar();
	}
}

int GameEngine::Run(HINSTANCE hInstance, cli::CommandLine const& cmdLine, int iCmdShow, unique_ptr<AbstractGame>&& game)
{

#if defined(DEBUG) | defined(_DEBUG)
	//notify user if heap is corrupt
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

	// Enable run-time memory leak check for debug builds.
	typedef HRESULT(__stdcall * fPtr)(const IID&, void**);
	HMODULE dxgidebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	IDXGIDebug* pDXGIDebug = nullptr;
	if (dxgidebug)
	{
		fPtr const dxgi_get_debug_interface = reinterpret_cast<fPtr>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface"));
		assert(dxgi_get_debug_interface);

		dxgi_get_debug_interface(__uuidof(IDXGIDebug), reinterpret_cast<void**>(&pDXGIDebug));
	}
#endif

	int result = 0;
	GameEngine::create();

	GameEngine::instance()->set_command_line(cmdLine);
	GameEngine::instance()->set_game(std::move(game));

	result = GameEngine::instance()->Run(hInstance, iCmdShow); // run the game engine and return the result

	// Shutdown the game engine to make sure there's no leaks left.
	GameEngine::shutdown();

#if defined(DEBUG) | defined(_DEBUG)
	if (pDXGIDebug)
	{
		pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	}
	Helpers::SafeRelease(pDXGIDebug);
#endif
	return result;
}

void RenderThread::run()
{
	_stage = Stage::Initialization;
	LOG_INFO(Graphics, "Launching render thread...");
	Perf::precise_sleep(2.0f);


	_stage = Stage::Running;

	u64 _frame = 0;
	while(is_running())
	{
		LOG_INFO(Graphics, "Frame  {}", _frame);

		Perf::precise_sleep(0.25f);
	
		++_frame;
	}
	_stage = Stage::Cleanup;
	LOG_INFO(Graphics, "Shutting down render thread...");
	Perf::precise_sleep(2.0f);

	_stage = Stage::Terminated;
}

static GlobalContext g_GlobalContext{};
GlobalContext* GetGlobalContext()
{
	return &g_GlobalContext;
}
