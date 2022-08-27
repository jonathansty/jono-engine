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

int g_DebugMode = 0;

static constexpr uint32_t max_task_threads = 8;

// Static task scheduler used for loading assets and executing multi threaded work loads
enki::TaskScheduler* GameEngine::s_TaskScheduler;

// Thread ID used to identify if we are on the main thread.
std::thread::id GameEngine::s_main_thread;

// Window procedure to forward events to the game engine
LRESULT CALLBACK GameEngine::WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route all Windows messages to the game engine
	return GameEngine::instance()->handle_event(hWindow, msg, wParam, lParam);
}

// #TODO: Remove this once full 2D graphics has been refactored into it's own context
#if FEATURE_D2D
using Graphics::bitmap_interpolation_mode;
#endif

GameEngine::GameEngine()
		: _hinstance(0)
		, _hwindow(NULL)
		, _icon(0)
		, _small_icon(0) //changed in june 2014, reset to false in dec 2014
		, _window_width(0)
		, _window_height(0)
		, _should_sleep(true)
		, _game(nullptr)
		, _can_paint(false)
		, _vsync_enabled(true)
		, _initialized(false)
		, _game_timer()
		, _default_font(nullptr)
		, _input_manager(nullptr)
		, _xaudio_system(nullptr)
		, _game_settings()
		, _physics_step_enabled(true)
		, _is_viewport_focused(true) // TODO: When implementing some kind of editor system this should be updating
		, _recreate_game_texture(false)
		, _recreate_swapchain(false)
		, _debug_physics_rendering(false)
		, _gravity(float2(0, 9.81))
		, _show_debuglog(true)
		, _show_viewport(true)
		, _show_imgui_demo(false)
		, _show_implot_demo(false)
		, _show_entity_editor(false)
		, _renderer(nullptr)
{
	// Seed the random number generator
	srand((unsigned int)(GetTickCount64()));
}

GameEngine::~GameEngine()
{
}

void GameEngine::set_game(unique_ptr<AbstractGame>&& gamePtr)
{
	_game = std::move(gamePtr);
}

void GameEngine::set_title(const string& titleRef)
{
	_title = titleRef;
}

int GameEngine::run(HINSTANCE hInstance, int iCmdShow)
{
	JONO_THREAD("MainThread");

	// Create the IO first as our logging depends on creating the right folder
	_platform_io = IO::create();
	IO::set(_platform_io);

	// Create all the singletons needed by the game, the game engine singleton is initialized from run_game
	Logger::create();
	ResourceLoader::create();

	// Then we initialize the logger as this might create a log file
	Logger::instance()->init();


	// Now we can start logging information and we mount our resources volume.
	LOG_INFO(IO, "Mounting resources directory.");
	_platform_io->mount("Resources");

	ASSERTMSG(_game, "No game has been setup! Make sure to first create a game instance before launching the engine!");
	_game->configure_engine(this->_engine_settings);

	// Validate engine settings
	ASSERTMSG(!(_engine_settings.d2d_use && (_engine_settings.d3d_use && _engine_settings.d3d_msaa_mode != MSAAMode::Off)), " Currently the engine does not support rendering D2D with MSAA because DrawText does not respond correctly!");

	s_main_thread = std::this_thread::get_id();

	// initialize d2d for WIC
	SUCCEEDED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));

	// Setup our default overlays
	_overlay_manager = std::make_shared<OverlayManager>();
	_metrics_overlay = new MetricsOverlay(true);

	_overlay_manager->register_overlay(_metrics_overlay);
	_overlay_manager->register_overlay(new RTTIDebugOverlay());
	_overlay_manager->register_overlay(new ImGuiDemoOverlay());
	_overlay_manager->register_overlay(new ImGuiAboutOverlay());

	// set the instance member variable of the game engine
	this->_hinstance = hInstance;

	// Initialize enkiTS
	s_TaskScheduler = Tasks::get_scheduler();
	Tasks::get_scheduler()->Initialize(max_task_threads);

	struct InitTask : enki::IPinnedTask
	{
		InitTask(uint32_t threadNum)
				: IPinnedTask(threadNum) {}

		void Execute() override
		{
			LOG_INFO(System, "Initializing Task thread {}.", threadNum);

			std::wstring name = fmt::format(L"TaskThread {}", threadNum);
			::SetThreadDescription(GetCurrentThread(), name.c_str());
			SUCCEEDED(::CoInitialize(NULL));
		}
	};

	std::vector<std::unique_ptr<InitTask>> tasks;
	for (uint32_t i = 0; i < s_TaskScheduler->GetNumTaskThreads(); ++i)
	{
		tasks.push_back(std::make_unique<InitTask>(i));
		s_TaskScheduler->AddPinnedTask(tasks[i].get());
	}
	s_TaskScheduler->RunPinnedTasks();
	s_TaskScheduler->WaitforAll();



	// Task Set dependency testing. 
	{
		struct TaskA : enki::ITaskSet
		{
			void ExecuteRange(enki::TaskSetPartition range, u32 threadNum) override
			{
				LOG_INFO(System, "TaskA::Started");
				//enki::TaskSet taskSet = enki::TaskSet(100,[](enki::TaskSetPartition range, u32 threadNum)
				//		{ 
				//		PWSTR data;
				//		::GetThreadDescription(GetCurrentThread(), &data);
				//		std::wstring wbuff = data;
				//		std::string threadName = std::string(wbuff.begin(), wbuff.end());
				//		LOG_INFO(System, "{}: TaskA Logging from lambda taskset!", threadName);
				//	});
				//s_TaskScheduler->AddTaskSetToPipe(&taskSet);
				//s_TaskScheduler->WaitforTask(&taskSet);
				LOG_INFO(System, "TaskA::Ended");
			}
		};

		struct TaskB : enki::ITaskSet
		{
			enki::Dependency m_Dependency;
			void ExecuteRange(enki::TaskSetPartition range, u32 threadNum) override
			{
				LOG_INFO(System, "TaskB::Started");
				enki::TaskSet taskSet = enki::TaskSet(
					[](enki::TaskSetPartition range, u32 threadNum){ 
						LOG_INFO(System, "TaskB Logging from lambda taskset!"); 
					}
				);
				s_TaskScheduler->AddTaskSetToPipe(&taskSet);
				s_TaskScheduler->WaitforTask(&taskSet);
				LOG_INFO(System, "TaskB::Ended");
			}
		};

		TaskA taskA;
		TaskB taskB;
		taskB.SetDependency(taskB.m_Dependency, &taskA);

		s_TaskScheduler->AddTaskSetToPipe(&taskA);
		s_TaskScheduler->WaitforTask(&taskB);
	}




	//Initialize the high precision timers
	_game_timer = make_unique<PrecisionTimer>();
	_game_timer->reset();

	_input_manager = make_unique<InputManager>();
	_input_manager->init();

	// Sound system
#if FEATURE_XAUDIO
	_xaudio_system = make_unique<XAudioSystem>();
	_xaudio_system->init();
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

	_renderer = std::make_shared<Graphics::Renderer>();
	_renderer->init(_engine_settings, _game_settings, _command_line);

	// Game Initialization
	_game->initialize(_game_settings);
	apply_settings(_game_settings);

	// Open the window
	if (!this->register_wnd_class())
	{
		MessageBoxA(NULL, "Register class failed", "error", MB_OK);
		return false;
	}
	if (!this->open_window(iCmdShow))
	{
		MessageBoxA(NULL, "Open window failed", "error", MB_OK);
		return false;
	}
	_renderer->init_for_hwnd(_hwindow);
	Graphics::init(_renderer->get_ctx());

	// Initialize the Graphics Engine
	d3d_init();

	TextureResource::initialise_default();

	LOG_INFO(System, "Initialising worlds...");
	{
		_world = std::make_shared<framework::World>();
		_world->init();

		_render_world = std::make_shared<RenderWorld>();
		_render_world->init();
	}
	LOG_INFO(System, "Finished initialising worlds.");


	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
	//ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

	Graphics::DeviceContext ctx = _renderer->get_ctx();
	ImGui_ImplWin32_Init(get_window());
	ImGui_ImplDX11_Init(ctx._device.Get(), ctx._ctx.Get());
#pragma region Box2D
	// Initialize Box2D
	// Define the gravity vector.
	b2Vec2 gravity((float)_gravity.x, (float)_gravity.y);

	// Construct a world object, which will hold and simulate the rigid bodies.
	_b2d_world = make_shared<b2World>(gravity);
	_b2d_contact_filter = make_shared<b2ContactFilter>();

	_b2d_world->SetContactFilter(_b2d_contact_filter.get());
	_b2d_world->SetContactListener(this);

#if FEATURE_D2D
	_b2d_debug_renderer.SetFlags(b2Draw::e_shapeBit);
	_b2d_debug_renderer.AppendFlags(b2Draw::e_centerOfMassBit);
	_b2d_debug_renderer.AppendFlags(b2Draw::e_jointBit);
	_b2d_debug_renderer.AppendFlags(b2Draw::e_pairBit);
	_b2d_world->SetDebugDraw(&_b2d_debug_renderer);
#endif
#pragma endregion

	// User defined functions for start of the game
	_game->start();


	// Initialize our performance tracking
	Perf::initialize(_renderer->get_raw_device());

	// Initialize our GPU timers
	for (u32 j = 0; j < std::size(m_GpuTimings); ++j)
	{
		m_GpuTimings[j] = Perf::Timer(_renderer->get_raw_device());
	}

	// Timer to track the elapsed time in the game
	PrecisionTimer full_frame_timer{};
	f64 time_elapsed = 0.0;
	f64 time_previous = 0.0;
	f64 time_lag = 0.0; 

	_running = true;
	while (_running)
	{
		JONO_FRAME("Frame");

		full_frame_timer.start();

		{
			JONO_EVENT("PollInput");

			Timer t{};
			t.Start();
			// Process all window messages
			MSG msg{};
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			t.Stop();
			_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::EventHandlingCPU, t.GetTimeInMS());
		}

		// Running might have been updated by the windows message loop. Handle this here.
		if (!_running)
		{
			break;
		}

		{


			// Execute the game simulation loop
			{
				JONO_EVENT("Simulation");
				f64 current = time_elapsed;
				f64 elapsed = current - time_previous; 
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::FrameTime, (float)(elapsed * 1000.0f));

				time_previous = current; 
				time_lag += elapsed;

				Timer t{};
				t.Start();
				while (time_lag >= _physics_timestep)
				{
					// Call the Game Tick method
					_game->tick(_physics_timestep);

					int32 velocityIterations = 6;
					int32 positionIterations = 2;
					if (_physics_step_enabled)
					{
						_b2d_world->Step((float)_physics_timestep, velocityIterations, positionIterations);
					}

					// Step generates contact lists, pass to Listeners and clear the vector
					CallListeners();
					time_lag -= _physics_timestep;

					// Input manager update takes care of swapping the state
					_input_manager->update();
				}
				t.Stop();
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::GameUpdateCPU, (float)t.GetTimeInMS());
			}


			Perf::begin_frame(_renderer->get_raw_device_context());
			if (_recreate_swapchain)
			{
				_renderer->resize_swapchain(_window_width, _window_height);
				_recreate_swapchain = false;
			}

			// Recreating the game viewport texture needs to happen before running IMGUI and the actual rendering
			{
				JONO_EVENT("DebugUI");
				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				build_ui();
				ImVec2 game_width = { get_width() / 2.0f, get_height() / 2.0f };
				ImGui::SetNextWindowSize(game_width, ImGuiCond_FirstUseEver);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
				ImGui::PopStyleVar(1);

				_game->debug_ui();
				_overlay_manager->render_overlay();

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

			if (Perf::collect_disjoint(_renderer->get_raw_device_context(), timestampDisjoint))
			{
				auto& timing_data = m_GpuTimings[current];
				f64 cpuTime;
				timing_data.flush(_renderer->get_raw_device_context(), start, end, cpuTime);

				double diff = (double)(end - start) / (double)timestampDisjoint.Frequency;
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::RenderGPU, (float)(diff * 1000.0));
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::RenderCPU, (float)(cpuTime * 1000.0));
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
			_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::PresentCPU, present_timer.get_delta_time() * 1000.0);
			if (_engine_settings.max_frame_time > 0.0)
			{
				f64 targetTimeMs = _engine_settings.max_frame_time;

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
	_game->end();
	_game.reset();

	// Make sure all tasks have finished before shutting down
	Tasks::get_scheduler()->WaitforAllAndShutdown();

	Perf::shutdown();

	ResourceLoader::instance()->unload_all();
	ResourceLoader::shutdown();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	// Teardown graphics resources and windows procedures
	{
		_default_font.reset();

		// Deinit our global graphics API before killing the game engine API
		Graphics::deinit();
		_renderer->deinit();
		_renderer = nullptr;

		// deinit the engine graphics layer
		d3d_deinit();

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
	static std::unique_ptr<Graphics::D2DRenderContext> s_context = nullptr;
	if (!s_context)
	{
		s_context = std::make_unique<Graphics::D2DRenderContext>( _renderer.get(), _renderer->get_raw_d2d_factory(), _renderer->get_2d_draw_ctx(), _renderer->get_2d_color_brush(), _default_font.get() );
	}
	Graphics::D2DRenderContext& context = *s_context;
	context.begin_paint(_renderer.get(), _renderer->get_raw_d2d_factory(), _renderer->get_2d_draw_ctx(), _renderer->get_2d_color_brush(), _default_font.get());
	_d2d_ctx = &context;

	auto size = this->get_viewport_size();

	_can_paint = true;
	// make sure tvp.Heighthe view matrix is taken in account
	context.set_world_matrix(float4x4::identity());

	{
		GPU_SCOPED_EVENT(_renderer->get_raw_annotation(), "D2D:Paint");
		_game->paint(context);
	}

	// draw Box2D debug rendering
	// http://www.iforce2d.net/b2dtut/debug-draw
	if (_debug_physics_rendering)
	{
		// dimming rect in screenspace
		context.set_world_matrix(float4x4::identity());
		float4x4 matView = context.get_view_matrix();
		context.set_view_matrix(float4x4::identity());
		context.set_color(MK_COLOR(0, 0, 0, 127));
		context.fill_rect(0, 0, get_width(), get_height());
		context.set_view_matrix(matView);

		_b2d_debug_renderer.set_draw_ctx(&context);
		_b2d_world->DebugDraw();
		_b2d_debug_renderer.set_draw_ctx(nullptr);
	}

	_can_paint = false;
	bool result = context.end_paint();
	_d2d_ctx = nullptr;

	// if drawing failed, terminate the game
	if (!result)
	{
		PostMessage(GameEngine::get_window(), WM_DESTROY, 0, 0);
	}
}
#endif

bool GameEngine::register_wnd_class()
{
	WNDCLASSEX wndclass;

	std::wstring title{ _title.begin(), _title.end() };
	// Create the window class for the main window
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = _hinstance;
	wndclass.hIcon = LoadIcon(_hinstance, MAKEINTRESOURCE(get_icon()));
	wndclass.hIconSm = LoadIcon(_hinstance, MAKEINTRESOURCE(get_small_icon()));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = title.c_str();

	// Register the window class
	if (!RegisterClassEx(&wndclass))
		return false;
	return true;
}

bool GameEngine::open_window(int iCmdShow)
{
	// Calculate the window size and position based upon the game size
	DWORD windowStyle = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	RECT R = { 0, 0, _window_width, _window_height };
	AdjustWindowRect(&R, windowStyle, false);
	int iWindowWidth = R.right - R.left;
	int iWindowHeight = R.bottom - R.top;
	int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2;
	int iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;

	std::wstring title = std::wstring(_title.begin(), _title.end());
	_hwindow = CreateWindow(title.c_str(), title.c_str(),
			windowStyle,
			iXWindowPos, iYWindowPos, iWindowWidth,
			iWindowHeight, NULL, NULL, _hinstance, NULL);

	if (!_hwindow)
		return false;

	// Show and update the window
	if (_game_settings.m_WindowFlags & GameSettings::WindowFlags::StartMaximized)
		iCmdShow = SW_SHOWMAXIMIZED;

	ShowWindow(_hwindow, iCmdShow);
	UpdateWindow(_hwindow);

	// Update size

	RECT r;
	::GetClientRect(_hwindow, &r);
	_window_width = r.right - r.left;
	_window_height = r.bottom - r.top;
	assert(_window_width > 0 && _window_height > 0);

	return true;
}

void GameEngine::quit_game()
{
	this->_running = false;
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
	_debug_physics_rendering = isEnabled;
}

HINSTANCE GameEngine::get_instance() const
{
	return _hinstance;
}

HWND GameEngine::get_window() const
{
	return _hwindow;
}

string GameEngine::get_title() const
{
	return _title;
}

WORD GameEngine::get_icon() const
{
	return _icon;
}

WORD GameEngine::get_small_icon() const
{
	return _small_icon;
}

ImVec2 GameEngine::get_window_size() const
{
	return { (float)_window_width, (float)_window_height };
}

ImVec2 GameEngine::get_viewport_size(int) const
{
	return { (float)_viewport_width, (float)_viewport_height };
}

ImVec2 GameEngine::get_viewport_pos(int id /*= 0*/) const
{
	RECT rect;
	GetWindowRect(get_window(), &rect);

	return {
		(float)_viewport_pos.x - rect.left, (float)_viewport_pos.y - rect.top
	};
}

int GameEngine::get_width() const
{
	return _window_width;
}

int GameEngine::get_height() const
{
	return _window_height;
}

bool GameEngine::get_sleep() const
{
	return _should_sleep ? true : false;
}

float2 GameEngine::get_mouse_pos_in_window() const
{
	RECT rect;
	if (GetWindowRect(get_window(), &rect))
	{
		return float2{ (float)_input_manager->get_mouse_position().x, (float)_input_manager->get_mouse_position().y } - float2(rect.left, rect.top);
	}
	return {};
}

float2 GameEngine::get_mouse_pos_in_viewport() const
{
	float2 tmp = float2(_input_manager->get_mouse_position());
	return float2{ (float)tmp.x, (float)tmp.y } - _viewport_pos;
}

unique_ptr<XAudioSystem> const& GameEngine::get_audio_system() const
{
	return _xaudio_system;
}

void GameEngine::set_icon(WORD wIcon)
{
	_icon = wIcon;
}

void GameEngine::set_small_icon(WORD wSmallIcon)
{
	_small_icon = wSmallIcon;
}

void GameEngine::set_width(int iWidth)
{
	_window_width = iWidth;
	assert(_window_width > 0);
}

void GameEngine::set_height(int iHeight)
{
	_window_height = iHeight;
	assert(_window_height > 0);
}

void GameEngine::set_physics_step(bool bEnabled)
{
	_physics_step_enabled = bEnabled;
}

bool GameEngine::is_viewport_focused() const
{
	return _is_viewport_focused;
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
	if (_game_timer == nullptr)
		return;

	_should_sleep = bSleep;
	if (bSleep)
	{
		_game_timer->stop();
	}
	else
	{
		_game_timer->start();
	}
}

void GameEngine::enable_vsync(bool bEnable)
{
	_vsync_enabled = bEnable;
}

void GameEngine::apply_settings(GameSettings& game_settings)
{
	enable_aa(_engine_settings.d2d_use_aa);

	set_width(game_settings.m_WindowWidth);
	set_height(game_settings.m_WindowHeight);
	set_title(game_settings.m_WindowTitle);
	enable_vsync(game_settings.m_WindowFlags & GameSettings::WindowFlags::EnableVSync);
}

void GameEngine::set_vsync(bool vsync)
{
	_vsync_enabled = vsync;
}

bool GameEngine::get_vsync()
{
	return _vsync_enabled;
}

std::shared_ptr<OverlayManager> const& GameEngine::get_overlay_manager() const
{
	return _overlay_manager;
}

// Input methods
bool GameEngine::is_key_down(int key) const
{
	return _input_manager->is_key_down((KeyCode)key);
}

bool GameEngine::is_key_pressed(int key) const
{
	return _input_manager->is_key_pressed((KeyCode)key);
}

bool GameEngine::is_key_released(int key) const
{
	return _input_manager->is_key_released((KeyCode)key);
}

bool GameEngine::is_mouse_button_down(int button) const
{
	return _input_manager->is_mouse_button_down(button);
}

bool GameEngine::is_mouse_button_pressed(int button) const
{
	return _input_manager->is_mouse_button_pressed(button);
}

bool GameEngine::is_mouse_button_released(int button) const
{
	return _input_manager->is_mouse_button_released(button);
}

LRESULT GameEngine::handle_event(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Get window rectangle and HDC
	RECT windowClientRect;
	GetClientRect(hWindow, &windowClientRect);

	RECT usedClientRect;
	usedClientRect.left = 0;
	usedClientRect.top = 0;
	usedClientRect.right = get_width();
	usedClientRect.bottom = get_height();

	// Route Windows messages to game engine member functions
	switch (msg)
	{
		case WM_CREATE:
			// Set the game window
			this->_hwindow = hWindow;
			return 0;
		case WM_SYSCOMMAND: // trapping this message prevents a freeze after the ALT key is released
			if (wParam == SC_KEYMENU)
				return 0; // see win32 API : WM_KEYDOWN
			else
				break;
		case WM_DESTROY:
			GameEngine::instance()->quit_game();
			return 0;

		case WM_SIZE:
			if (wParam == SIZE_MAXIMIZED)
			{
				//If you have changed certain window data using SetWindowLong, you must call SetWindowPos for the changes to take effect.
				SetWindowPos(_hwindow, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			RECT r;
			::GetClientRect(_hwindow, &r);
			_window_width = r.right - r.left;
			_window_height = r.bottom - r.top;

			this->_recreate_swapchain = true;
			return 0;

			//case WM_KEYUP:
			//	m_GamePtr->KeyPressed((TCHAR)wParam);
			//	return 0;

		// Posted to the window with the keyboard focus when a nonsystem key is pressed. A nonsystem key is a key that is pressed when the ALT key is not pressed.
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (msg == WM_KEYUP && wParam == VK_F9)
			{
				_overlay_manager->set_visible(!_overlay_manager->get_visible());
			}
			break;
	}

	bool handled = false;

	if (ImGui::GetCurrentContext() != nullptr)
	{
		//bool bWantImGuiCapture = ImGui::GetIO().WantCaptureKeyboard ||
		//						 ImGui::GetIO().WantCaptureMouse;

		//if (bWantImGuiCapture)
		{
			extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
			if (LRESULT v = ImGui_ImplWin32_WndProcHandler(hWindow, msg, wParam, lParam); v != 0)
			{
				handled |= (v != 0);
			}
		}
	}

	if (!handled)
	{
		// Input manager doesn't consume the inputs
		handled |= _input_manager->handle_events(msg, wParam, lParam);
	}

	if (handled)
		return 0;

	return DefWindowProc(hWindow, msg, wParam, lParam);
}

//
//  This method creates resources which are bound to a particular
//  Direct3D device. It's all centralized here, in case the resources
//  need to be recreated in case of Direct3D device loss (eg. display
//  change, remoting, removal of video card, etc).
//
void GameEngine::d3d_init()
{
	//Create a Font
	RECT r{};
	if (GetWindowRect(_hwindow, &r))
	{
		_viewport_width = r.right - r.left;
		_viewport_height = r.bottom - r.top;
		_viewport_pos = { 0.0f,0.0f };
	}
	_default_font = make_shared<Font>("Consolas", 12.0f);
	_initialized = true;
}

//
//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
//
void GameEngine::d3d_deinit()
{
	_initialized = false;
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
			_b2d_begin_contact_data.push_back(contactData);
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
			_b2d_begin_contact_data.push_back(contactData);
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
			_b2d_end_contact_data.push_back(contactData);
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
			_b2d_end_contact_data.push_back(contactData);
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
		_b2d_impulse_data.push_back(impulseData);
}

void GameEngine::CallListeners()
{
	// begin contact
	for (size_t i = 0; i < _b2d_begin_contact_data.size(); i++)
	{
		ContactListener* contactListenerPtr = reinterpret_cast<ContactListener*>(_b2d_begin_contact_data[i].contactListenerPtr);
		contactListenerPtr->BeginContact(
				reinterpret_cast<PhysicsActor*>(_b2d_begin_contact_data[i].actThisPtr),
				reinterpret_cast<PhysicsActor*>(_b2d_begin_contact_data[i].actOtherPtr));
	}
	_b2d_begin_contact_data.clear();

	// end contact
	for (size_t i = 0; i < _b2d_end_contact_data.size(); i++)
	{
		ContactListener* contactListenerPtr = reinterpret_cast<ContactListener*>(_b2d_end_contact_data[i].contactListenerPtr);
		contactListenerPtr->EndContact(
				reinterpret_cast<PhysicsActor*>(_b2d_end_contact_data[i].actThisPtr),
				reinterpret_cast<PhysicsActor*>(_b2d_end_contact_data[i].actOtherPtr));
	}
	_b2d_end_contact_data.clear();

	// impulses
	for (size_t i = 0; i < _b2d_impulse_data.size(); i++)
	{
		ContactListener* contactListenerAPtr = reinterpret_cast<ContactListener*>(_b2d_impulse_data[i].contactListenerAPtr);
		ContactListener* contactListenerBPtr = reinterpret_cast<ContactListener*>(_b2d_impulse_data[i].contactListenerBPtr);
		if (contactListenerAPtr != nullptr)
			contactListenerAPtr->ContactImpulse(reinterpret_cast<PhysicsActor*>(_b2d_impulse_data[i].actAPtr), _b2d_impulse_data[i].impulseA);
		if (contactListenerBPtr != nullptr)
			contactListenerBPtr->ContactImpulse(reinterpret_cast<PhysicsActor*>(_b2d_impulse_data[i].actBPtr), _b2d_impulse_data[i].impulseB);
	}
	_b2d_impulse_data.clear();
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
	_renderer->render_view(_render_world, pass);
}

void GameEngine::build_ui()
{
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
	auto d3d_annotation = _renderer->get_raw_annotation();
	GPU_SCOPED_EVENT(d3d_annotation, L"Frame");

	size_t idx = Perf::get_current_frame_resource_index();

	// Begin frame gpu timer
	auto& timer = m_GpuTimings[idx];
	timer.begin(_renderer->get_raw_device_context());

	_renderer->begin_frame();

	// Render 3D before 2D
	if (_engine_settings.d3d_use)
	{
		_renderer->pre_render(_render_world);

		GPU_SCOPED_EVENT(d3d_annotation, L"Render");
		// Render the shadows
		{
			_renderer->render_shadow_pass(_render_world);
		}

		{
			GPU_SCOPED_EVENT(d3d_annotation, L"Main");
			_renderer->render_zprepass(_render_world);
			_renderer->render_opaque_pass(_render_world);
		}
	}

	// Render Direct2D to the swapchain
	if (_engine_settings.d2d_use)
	{
#if FEATURE_D2D
		d2d_render();
#else
		LOG_ERROR(System, "Trying to use D2D but the build isn't compiled with D2D enabled!");
		DebugBreak();
#endif
	}

	_renderer->render_post(_render_world, _overlay_manager);
	_renderer->end_frame();
}

void GameEngine::present()
{
	JONO_EVENT();
	auto d3d_ctx = _renderer->get_raw_device_context();
	auto d3d_annotation = _renderer->get_raw_annotation();
	auto d3d_swapchain = _renderer->get_raw_swapchain();

	size_t idx = Perf::get_current_frame_resource_index();

	// Present, 
	GPU_MARKER(d3d_annotation, L"DrawEnd");
	u32 flags = 0;
	if (!_vsync_enabled)
	{
		flags |= DXGI_PRESENT_ALLOW_TEARING;
	}
	d3d_swapchain->Present(_vsync_enabled ? 1 : 0, flags);

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

		_is_viewport_focused = ImGui::IsWindowHovered();

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
			_viewport_width = (u32)s_vp_size.x;
			_viewport_height = (u32)s_vp_size.y;
			_viewport_pos = float2(s_vp_pos.x, s_vp_pos.y);

			_renderer->update_viewport(static_cast<u32>(vMin.x), static_cast<u32>(vMin.y), static_cast<u32>(s_vp_size.x), static_cast<u32>(s_vp_size.y));
		}

		// Draw the actual scene image
		ImVec2 max_uv = {};
		max_uv.x = s_vp_size.x / get_width();
		max_uv.y = s_vp_size.y / get_height();

		_viewport_pos = float2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		ImGui::Image(_renderer->get_raw_output_non_msaa_srv(), s_vp_size, ImVec2(0, 0), max_uv);


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

int GameEngine::run_game(HINSTANCE hInstance, cli::CommandLine const& cmdLine, int iCmdShow, unique_ptr<AbstractGame>&& game)
{
	GameEngine::create();

#if defined(DEBUG) | defined(_DEBUG)
	//notify user if heap is corrupt
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

	// Enable run-time memory leak check for debug builds.
	typedef HRESULT(__stdcall * fPtr)(const IID&, void**);
	HMODULE const h_dll = GetModuleHandleW(L"dxgidebug.dll");
	IDXGIDebug* pDXGIDebug = nullptr;
	if (h_dll)
	{
		fPtr const dxgi_get_debug_interface = reinterpret_cast<fPtr>(GetProcAddress(h_dll, "DXGIGetDebugInterface"));
		assert(dxgi_get_debug_interface);

		dxgi_get_debug_interface(__uuidof(IDXGIDebug), reinterpret_cast<void**>(&pDXGIDebug));
	}
#endif

	int result = 0;

	GameEngine::instance()->set_command_line(cmdLine);
	GameEngine::instance()->set_game(std::move(game));

	result = GameEngine::instance()->run(hInstance, iCmdShow); // run the game engine and return the result

	// Shutdown the game engine to make sure there's no leaks left.
	GameEngine::shutdown();

#if defined(DEBUG) | defined(_DEBUG)
	if (pDXGIDebug)
	{
		pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
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

