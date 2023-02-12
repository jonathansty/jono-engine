#include "engine.pch.h"

#include "AbstractGame.h"
#include "ContactListener.h"
#include "GameEngine.h"

#include "Debug/ImGuiOverlays.h"
#include "Debug/MetricsOverlay.h"
#include "Debug/RTTIDebugOverlay.h"

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

#include "Types/TypeManager.h"
#include "EngineLoop.h"

int g_DebugMode = 0;

static constexpr uint32_t max_task_threads = 8;

// Static task scheduler used for loading assets and executing multi threaded work loads
enki::TaskScheduler* GameEngine::s_TaskScheduler;

// Thread ID used to identify if we are on the main thread.
std::thread::id GameEngine::s_MainThreadID;

// #TODO: Remove this once full 2D graphics has been refactored into it's own context
#if FEATURE_D2D
using Graphics::bitmap_interpolation_mode;
#endif

static constexpr Shaders::float2 s_Gravity = Shaders::float2(0.0f, -9.81f);

GameEngine::GameEngine()
	: m_Window(nullptr)
	, m_GpuTimings()
	, m_CommandLine()
	, m_Title()
	, m_Icon()
	, m_SmallIcon()
	, m_WindowWidth(0)
	, m_WindowHeight(0)
	, m_ViewportWidth(0)
	, m_ViewportHeight(0)
	, m_ViewportPos()
	, m_ViewportImGuiID()
	, m_DockImGuiID()
	, m_DefaultFont(nullptr)
	, m_Box2DWorld(nullptr)
	, m_Box2DContactFilter(nullptr)
	, m_Box2DTime(0.0)
#if FEATURE_D2D
	, m_Box2DDebugRenderer()
#endif
	, m_Box2DBeginContactData()
	, m_Box2DEndContactData()
	, m_Box2DImpulseData()
	, m_Gravity(s_Gravity)
	, m_FrameTimer(nullptr)
	, m_InputManager(nullptr)
	, m_AudioSystem(nullptr)
	, m_MetricsOverlay(nullptr)
	, m_OverlayManager(nullptr)
	, m_GameCfg()
	, m_EngineCfg()
	, m_PlatformIO(nullptr)
	, m_RenderWorld(nullptr)
	, m_World(nullptr)
	, m_Game(nullptr)
	, m_IsRunning(false)
	, m_CanPaint2D(false)
	, m_ViewportIsFocused(false)
	, m_VSyncEnabled(false)
	, m_DebugPhysicsRendering(false)
	, m_RecreateGameTextureRequested(false)
	, m_RecreateSwapchainRequested(false)
	, m_PhysicsStepEnabled(false)
	, m_ShowDebugLog(true)
	, m_ShowViewport(true)
	, m_ShowImguiDemo(false)
	, m_ShowImplotDemo(false)
	, m_ShowEntityEditor(false)
	, m_Renderer(nullptr)
	, m_SignalGraphicsToMain({0})
	, m_SignalMainToGraphics({0})
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

	ASSERT(!GetGlobalContext()->m_Game);
	GetGlobalContext()->m_Game = m_Game.get();

}

void GameEngine::set_title(const string& titleRef)
{
	m_Title = titleRef;
}

bool GameEngine::Startup()
{
	JONO_THREAD("MainThread");

	InitSubSystems();

	if (m_Game)
	{
		ASSERTMSG(m_Game, "No game has been setup! Make sure to first create a game instance before launching the engine!");
		if (m_Game)
		{
			MEMORY_TAG(MemoryCategory::Game);
			m_Game->ConfigureEngine(this->m_EngineCfg);
		}
	}

	// Validate engine settings
	ASSERTMSG(!(m_EngineCfg.m_UseD2D && (m_EngineCfg.m_UseD3D && m_EngineCfg.m_MSAA != MSAAMode::Off)), " Currently the engine does not support rendering D2D with MSAA because DrawText does not respond correctly!");

	s_MainThreadID = std::this_thread::get_id();

	// initialize d2d for WIC
	SUCCEEDED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));

	// Setup our default overlays
	m_OverlayManager = std::make_shared<OverlayManager>();
	m_MetricsOverlay = new MetricsOverlay(true);

	m_OverlayManager->register_overlay(m_MetricsOverlay);
	m_OverlayManager->register_overlay(new RTTIDebugOverlay());
	m_OverlayManager->register_overlay(new ImGuiDemoOverlay());
	m_OverlayManager->register_overlay(new ImGuiAboutOverlay());

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

	// Initialize the high precision timers
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

	// Kick-off the graphics thread
	m_GraphicsThread.Execute();

	m_Renderer = SharedPtr(new Graphics::Renderer());
	m_Renderer->Init(m_EngineCfg, m_GameCfg, m_CommandLine);

	// Game Initialization
	if (m_Game)
	{
		MEMORY_TAG(MemoryCategory::Game);
		m_Game->ConfigureGame(m_GameCfg);
	}
	apply_settings(m_GameCfg);

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
	}

	SDL_Init(SDL_INIT_EVERYTHING);

	if (!this->InitWindow(0))
	{
		MessageBoxA(NULL, "Open window failed", "error", MB_OK);
		return false;
	}

	m_Renderer->InitForWindow(m_Window);
	Graphics::init(m_Renderer->get_ctx());

	m_ViewportWidth = m_Renderer->GetDrawableWidth();
	m_ViewportHeight = m_Renderer->GetDrawableHeight();
	m_ViewportPos = { 0.0f, 0.0f };

	//m_DefaultFont = make_shared<Font>("Consolas", 12.0f);

	TextureHandle::init_default();

	LOG_INFO(System, "Initialising worlds...");
	{
		m_World = std::make_shared<framework::World>();
		m_World->init();

		m_RenderWorld = std::make_shared<RenderWorld>();
		m_RenderWorld->Init();
	}
	LOG_INFO(System, "Finished initialising worlds.");

		{
		// ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
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
		m_Game->OnStartup();
	}

	// Ensure world has default setup
	if (!get_render_world()->get_view_camera())
	{
		RenderWorldCameraRef camera = get_render_world()->create_camera();
		ImVec2 size = GameEngine::instance()->GetViewportSize();
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

	// Wait until graphics stage is initialized
	m_GraphicsThread.WaitForStage(GraphicsThread::Stage::Running);
	m_SignalGraphicsToMain.release();

	m_IsRunning = true;
	m_TimeAccum = 0.0;
	m_TimeT = 0.0;


	return true;
}

void GameEngine::Update(f64 dt)
{
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
		m_GraphicsThread.Terminate();
		m_SignalMainToGraphics.release();
		return;
	}

	{
		// Execute the game simulation loop
		{
			JONO_EVENT("Simulation");
			f64 elapsed = dt;
			m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::FrameTime, (float)(elapsed * 1000.0f));

			m_TimeAccum += elapsed;
			m_TimeT = 0.0;

			Timer t{};
			t.Start();
			while (m_TimeAccum >= c_FixedPhysicsTimestep)
			{
				// Call the Game Tick method
				if (m_Game)
				{
					MEMORY_TAG(MemoryCategory::Game);
					m_Game->OnUpdate(c_FixedPhysicsTimestep);
				}

				int32 velocityIterations = 6;
				int32 positionIterations = 2;
				if (m_PhysicsStepEnabled)
				{
					m_Box2DWorld->Step((float)c_FixedPhysicsTimestep, velocityIterations, positionIterations);
				}

				// Step generates contact lists, pass to Listeners and clear the vector
				CallListeners();
				m_TimeAccum -= c_FixedPhysicsTimestep;
				m_TimeT += c_FixedPhysicsTimestep;

				// Input manager update takes care of swapping the state
				m_InputManager->update();
			}
			t.Stop();
			m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::GameUpdateCPU, (float)t.GetTimeInMS());
		}

		// Run 2D rendering on the app thread, merely populates the 2D drawing data
		if (m_EngineCfg.m_UseD2D && !m_RecreateSwapchainRequested)
		{
			RenderD2D();
		}

		// Recreating the game viewport texture needs to happen before running IMGUI and the actual rendering
		{
			JONO_EVENT("DebugUI");
			ImGui_ImplDX11_NewFrame();

			// ImVec2 displaySize = { (float)m_Renderer->GetDrawableWidth(), (float)m_Renderer->GetDrawableHeight() };
			ImGui_ImplSDL2_NewFrame(nullptr);

			ImGui::NewFrame();

			BuildEditorUI();

			if (m_Game)
			{
				MEMORY_TAG(MemoryCategory::Debug);
				m_Game->OnDebugUI();
			}
			m_OverlayManager->RenderOverlay();

			ImGui::EndFrame();
			ImGui::UpdatePlatformWindows();
			ImGui::Render();
		}
	}

	ResourceLoader::instance()->update();

	// First sync our game update to the RT
	m_SignalGraphicsToMain.acquire();
	Sync();
	m_SignalMainToGraphics.release();
}

void GameEngine::Shutdown()
{
	m_GraphicsThread.Terminate();
	m_GraphicsThread.WaitForStage(GraphicsThread::Stage::Terminated);
	m_GraphicsThread.Join();

	// User defined code for exiting the game
	if (m_Game)
	{
		MEMORY_TAG(MemoryCategory::Game);
		m_Game->OnShutdown();
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

        GetRI()->Shutdown();
        delete GetRI();


		::CoUninitialize();
	}
}

struct EditorWindowData
{
	ImVec2ih position;
	ImVec2ih size;

	bool maximized = false;
};
EditorWindowData g_WindowData;

bool GameEngine::InitSubSystems()
{
	// Create the IO first as our logging depends on creating the right folder
	{
		m_PlatformIO = IO::create();
		IO::set(m_PlatformIO);

		// Now we can start logging information and we mount our resources volume.
		m_PlatformIO->Mount("Resources");

		GetGlobalContext()->m_PlatformIO = m_PlatformIO.get();
	}

	Logger::create();
	Logger::instance()->init();

	// Load the engine config to decide what other sub systems are needed
	{
		constexpr char const* c_ConfigPath = "res:/Config/Engine.cfg";
		constexpr char const* c_GameConfigPath = "res:/Config/Game.cfg";

		std::string configPath = m_PlatformIO->ResolvePath(c_ConfigPath);
		if (IO::IFileRef file = m_PlatformIO->OpenFile(configPath.c_str(), IO::Mode::Read); file)
		{
			u32 fileSize = (u32)file->GetSize();
			char* data = new char[fileSize];

			u32 bytesRead = file->read(data, fileSize);

			SharedPtr<IFileStream> readStream = SharedPtr<IFileStream>(new YamlStream(data, bytesRead));
			TypeManager* manager = GetGlobalContext()->m_TypeManager;
			manager->SerializeObject("/Types/Core/EngineCfg", &m_EngineCfg, readStream.Get());
		}

		std::string gameConfigPath = m_PlatformIO->ResolvePath(c_GameConfigPath);
		if (IO::IFileRef file = m_PlatformIO->OpenFile(gameConfigPath.c_str(), IO::Mode::Read); file)
		{
			u32 fileSize = (u32)file->GetSize();
			char* data = new char[fileSize];

			u32 bytesRead = file->read(data, fileSize);

			SharedPtr<IFileStream> readStream = SharedPtr<IFileStream>(new YamlStream(data, bytesRead));
			TypeManager* manager = GetGlobalContext()->m_TypeManager;
			manager->SerializeObject("/Types/Core/GameCfg", &m_GameCfg, readStream.Get());
		}

	}


	ResourceLoader::create();

	// Initialize enkiTS
	GlobalContext* globalContext = GetGlobalContext();
	ASSERT(!globalContext->m_TaskScheduler);
	globalContext->m_TaskScheduler = Tasks::get_scheduler();
	globalContext->m_TaskScheduler->Initialize();

	return true;
}

bool GameEngine::InitWindow(int iCmdShow)
{
	// Calculate the window size and position based upon the game size
	int iWindowWidth = m_WindowWidth;
	int iWindowHeight = m_WindowHeight;
	int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2;
	int iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;

	g_WindowData.position = ImVec2ih((short)iXWindowPos, (short)iYWindowPos);
	g_WindowData.size = ImVec2ih((short)iWindowWidth, (short)iWindowHeight);
	g_WindowData.maximized = false;

	// Setup custom handler for IMGUI to store our window dimensions, position and state
	// Then we trigger a load of the config manually to allow us to load our editor data.
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiSettingsHandler handler;
		handler.TypeName = "Editor";
		handler.TypeHash = ImHashStr("Editor");
		handler.UserData = this;
		handler.ReadOpenFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) -> void*
		{
			return &g_WindowData;
		};
		handler.ReadLineFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line)
		{
			EditorWindowData* data = (EditorWindowData*)entry;
			int x, y;
			if (sscanf_s(line, "Pos=%i,%i", &x, &y) == 2)
			{
				g_WindowData.position = ImVec2ih((short)x, (short)y);
			}
			else if (sscanf_s(line, "Size=%i,%i", &x, &y) == 2)
			{
				g_WindowData.size = ImVec2ih((short)x, (short)y);
			}
			else if (sscanf_s(line, "Maximized=%i", &x) == 1)
			{
				g_WindowData.maximized = (bool)x;
			};

		};
		handler.WriteAllFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
		{
			const char* settings_name = "WindowData";
			{
				buf->appendf("[%s][%s]\n", handler->TypeName, settings_name);
				buf->appendf("Pos=%d,%d\n", g_WindowData.position.x, g_WindowData.position.y);
				buf->appendf("Size=%d,%d\n", g_WindowData.size.x, g_WindowData.size.y);
				buf->appendf("Maximized=%d\n", g_WindowData.maximized ? 1 : 0);
				buf->append("\n");
			}
		};

		ImGui::GetCurrentContext()->SettingsHandlers.push_back(handler);

		if (ImGui::GetIO().IniFilename)
		{
			ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
		}
	}

	u32 flags = SDL_WINDOW_RESIZABLE;
	if ((m_GameCfg.m_WindowFlags & GameCfg::WindowFlags::StartMaximized) || g_WindowData.maximized)
	{
		flags |= SDL_WINDOW_MAXIMIZED;
	}

	std::wstring title = std::wstring(m_Title.begin(), m_Title.end());
	m_Window = SDL_CreateWindow(m_Title.c_str(), g_WindowData.position.x, g_WindowData.position.y, g_WindowData.size.x, g_WindowData.size.y, flags);
	if (!m_Window)
	{
		FAILMSG("Failed to create the SDL window.");
		return false;
	}

	return true;
}

void GameEngine::DeInitSubSystems()
{
	TypeManager::shutdown();
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

ImVec2 GameEngine::GetWindowSize() const
{
	return { (float)m_WindowWidth, (float)m_WindowHeight };
}

ImVec2 GameEngine::GetViewportSize(int) const
{
	return { (float)m_ViewportWidth, (float)m_ViewportHeight };
}

ImVec2 GameEngine::get_viewport_pos(int id /*= 0*/) const
{
	SDL_Rect const* rect = SDL_GetWindowMouseRect(m_Window);
	return ImVec2((float)m_ViewportPos.x - rect->x, (float)m_ViewportPos.y - rect->y);
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

bool GameEngine::WantCaptureMouse() const
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool GameEngine::WantCaptureKeyboard() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

float2 GameEngine::get_mouse_pos_in_window() const
{
	SDL_Rect const* rect = SDL_GetWindowMouseRect(m_Window);
	return float2{ (float)m_InputManager->get_mouse_position().x, (float)m_InputManager->get_mouse_position().y } - float2(rect->x, rect->y);
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
	enable_aa(m_EngineCfg.m_UseD2DAA);

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
	// Route Windows messages to game engine member functions
	switch (e.type)
	{
		case SDL_WINDOWEVENT:
		{
			SDL_GetWindowSize(m_Window, &m_WindowWidth, &m_WindowHeight);

			int x, y;
			SDL_GetWindowPosition(m_Window, &x, &y);

			u32 flags = SDL_GetWindowFlags(m_Window);

			g_WindowData.size = ImVec2ih((short)m_WindowWidth, (short)m_WindowHeight);
			g_WindowData.position = ImVec2ih((short)x, (short)y);
			g_WindowData.maximized = false;
			if (flags & SDL_WINDOW_MAXIMIZED)
			{
				g_WindowData.maximized = true;
			}


			if (e.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				GameEngine::instance()->Quit();
				return;
			}
			else if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED)
			{
				this->m_RecreateSwapchainRequested = true;
			}
			else if (e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				SDL_GetWindowSize(m_Window, &m_WindowWidth, &m_WindowHeight);
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

void GameEngine::Sync()
{
	m_GraphicsThread.Sync();

	// Update the old app thread parameters
	m_RecreateSwapchainRequested = false;
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

void GameEngine::BuildEditorUI()
{
	MEMORY_TAG(MemoryCategory::Debug);

	static ImGuiID s_ViewportDockID;
	static ImGuiID s_PropertyDockID;
	static ImGuiID s_DebugLogDockID;
	static bool s_DockInitialized = false;
	static ImGuiID dockSpaceID = ImGui::GetID("Dockspace##Root");


	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	{
		ImGui::SetNextWindowPos(viewport->WorkPos);

		ImVec2 workSize = ImVec2( viewport->WorkSize.x, viewport->WorkSize.y );
		ImGui::SetNextWindowSize(workSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	ImGui::Begin(s_RootImguiID, nullptr, window_flags);
	ImGui::PopStyleVar(3);

	if (ImGui::DockBuilderGetNode(dockSpaceID) == nullptr)
	{
		ImGui::DockBuilderRemoveNode(dockSpaceID);
		ImGui::DockBuilderAddNode(dockSpaceID, (u32)ImGuiDockNodeFlags_DockSpace | (u32)ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::DockBuilderSetNodeSize(dockSpaceID, viewport->Size);

		ImGuiID dockMain = dockSpaceID;
		s_DebugLogDockID = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.25f, nullptr, &dockMain);
		s_PropertyDockID = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.25f, nullptr, &dockMain);
		s_ViewportDockID = dockMain;

		m_PropertyDockID = s_PropertyDockID;

		ImGui::DockBuilderFinish(dockMain);
	}
	ImGui::DockSpace(dockSpaceID);

	BuildMenuBarUI();
	BuildViewportUI(&s_ViewportDockID);
	BuildDebugLogUI(&s_DebugLogDockID);

	if (m_ShowImplotDemo)
	{
		ImPlot::ShowDemoWindow(&m_ShowImplotDemo);
	}

	ImGui::End();
}



void GameEngine::BuildDebugLogUI(ImGuiID* dockID)
{
	if (!m_ShowDebugLog)
	{
		return;
	}

	if (dockID && (*dockID != 0))
	{
		ImGui::SetNextWindowDockID(*dockID, ImGuiCond_Once);
	}

	if (ImGui::Begin("Output Log", &m_ShowDebugLog, 0))
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

void GameEngine::BuildViewportUI(ImGuiID* dockID)
{
	if (!m_ShowViewport)
	{
		return;
	}

	if (dockID && (*dockID != 0))
	{
		ImGui::SetNextWindowDockID(*dockID, ImGuiCond_Once);
	}
	if (ImGui::Begin("Viewport##GameViewport", &m_ShowViewport))
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

			m_Renderer->UpdateViewport(static_cast<u32>(vMin.x), static_cast<u32>(vMin.y), static_cast<u32>(s_vp_size.x), static_cast<u32>(s_vp_size.y));
		}

		// Draw the actual scene image
		ImVec2 max_uv = {};
		max_uv.x = s_vp_size.x / m_Renderer->GetDrawableWidth();
		max_uv.y = s_vp_size.y / m_Renderer->GetDrawableHeight();

		m_ViewportPos = float2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		ImGui::Image(m_Renderer->get_raw_output_non_msaa_srv(), s_vp_size, ImVec2(0, 0), max_uv);

		//ImGuizmo::SetDrawlist();
		//ImGuizmo::SetRect(_viewport_pos.x, _viewport_pos.y, float1(_viewport_width), float1(_viewport_height));

		get_overlay_manager()->RenderViewport();

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

void GameEngine::BuildMenuBarUI()
{
	if (ImGui::BeginMenuBar())
	{
		if (_build_menu)
		{
			_build_menu(BuildMenuOrder::First);
		}

#if 0
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
#endif

		if (ImGui::BeginMenu("Windows"))
		{
			if (ImGui::MenuItem("[DEMO] ImPlot"))
			{
				m_ShowImplotDemo = !m_ShowImplotDemo;
			}

			if (ImGui::MenuItem("Debug Log"))
			{
				m_ShowDebugLog = !m_ShowDebugLog;
			}
			if (ImGui::MenuItem("Viewport"))
			{
				m_ShowViewport = !m_ShowViewport;
			}
			if (ImGui::MenuItem("Entity Editor"))
			{
				m_ShowEntityEditor = !m_ShowEntityEditor;
				if (auto editor = get_overlay_manager()->get_overlay("EntityEditor"))
				{
					get_overlay_manager()->get_overlay("EntityDebugOverlay")->set_visible(m_ShowEntityEditor);
				}
			}


			ImGui::EndMenu();
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
	// notify user if heap is corrupt
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

    EngineLoop loop = EngineLoop(game->GetType()->m_Path);
    result = loop.Run(cmdLine);

	// #TODO: Pass command line to engine loop
	// GameEngine::instance()->set_command_line(cmdLine);

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

void GameEngine::RenderD2D()
{
	//#TODO: Revisit thread safety here. Remove engine->m_DefaultFont

	GameEngine* engine = this;
	Graphics::Renderer* renderer = engine->m_Renderer.get();

	// Plan to modernize the 2D and deprecate Direct2D
	// 1. Collect all the draw commands in buffers and capture the required data
	// 2. during end_paint 'flush' draw commands and create required vertex buffers
	// 3. Execute each draw command binding the right buffers and views
	if(!m_D2DRenderContext.IsValid())
	{
		m_D2DRenderContext = SharedPtr(new Graphics::D2DRenderContext());
	}

	Graphics::D2DRenderContext& context = *m_D2DRenderContext;
	context.begin_paint(renderer, renderer->get_raw_d2d_factory(), renderer->get_2d_draw_ctx(), renderer->get_2d_color_brush(), engine->m_DefaultFont.get());

	uint2 size = uint2(GetViewportSize().x, GetViewportSize().y);

	// make sure tvp.Heighthe view matrix is taken in account
	context.set_world_matrix(float4x4::identity());

	{
		if (engine->m_Game)
		{
			MEMORY_TAG(MemoryCategory::Game);
			engine->m_Game->OnPaint2D(context);
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
		context.fill_rect(0, 0, (int)GetWindowSize().x, (int)GetWindowSize().y);
		context.set_view_matrix(matView);

		engine->m_Box2DDebugRenderer.set_draw_ctx(&context);
		engine->m_Box2DWorld->DebugDraw();
		engine->m_Box2DDebugRenderer.set_draw_ctx(nullptr);
	}

	bool result = context.end_paint();

	// if drawing failed, terminate the game
	if (!result)
	{
		FAILMSG("Something went wrong drawing D2D elements. Crashing the game...");
		//this->Terminate();
		engine->m_IsRunning = false;
	}
}

#ifdef ENGINE_DLL
BOOL WINAPI DllMain(
	HINSTANCE hInstDLL,
	DWORD reason,
	LPVOID lpReserved
)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
			OutputDebugString(L"GameEngine::ProcessAttach\n");
			break;
		case DLL_THREAD_ATTACH:
			OutputDebugString(L"GameEngine::ThreadAttach\n");
			break;
		case DLL_THREAD_DETACH:
			OutputDebugString(L"GameEngine::ThreadDetach\n");
			break;
		case DLL_PROCESS_DETACH:
			OutputDebugString(L"GameEngine::ProcessDetach\n");
			break;
	
	}

	return TRUE;
}
#endif



