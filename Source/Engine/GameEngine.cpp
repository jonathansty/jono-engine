#include "engine.pch.h"

#include "GameEngine.h"
#include "ContactListener.h"
#include "AbstractGame.h"

#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/RTTIDebugOverlay.h"
#include "debug_overlays/ImGuiOverlays.h"

#include "core/ResourceLoader.h"

#include "InputManager.h"
#include "PrecisionTimer.h"
#include "AudioSystem.h"
#include "Font.h"
#include "Graphics/Graphics.h"

#include "Engine/Core/TextureResource.h"
#include "Engine/Core/MaterialResource.h"
#include "Engine/Core/Material.h"
#include "Core/Logging.h"


static constexpr uint32_t max_task_threads = 4;

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
using graphics::bitmap_interpolation_mode;
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
	, _dxgi_factory(nullptr)
	, _d3d_device(nullptr)
	, _d3d_device_ctx(nullptr)
	, _dxgi_swapchain(nullptr)
	, _d2d_factory(nullptr)
	, _wic_factory(nullptr)
	, _d2d_rt(nullptr)
	, _dwrite_factory(nullptr)
	, _game_timer()
	, _color_brush(nullptr)
	, _aa_desc({ 1,0 })
	, _default_font(nullptr)
	, _input_manager(nullptr)
	, _xaudio_system(nullptr)
	, _game_settings()
	, _physics_step_enabled(true)
	, _is_viewport_focused(true) // TODO: When implementing some kind of editor system this should be updating
	, _recreate_game_texture(false)
	, _recreate_swapchain(false)
	, _debug_physics_rendering(false)
	, _d3d_backbuffer_view(nullptr)
	, _gravity(float2(0, 9.81))
	, _d3d_backbuffer_srv(nullptr)
	, _d3d_output_depth(nullptr)
	, _d3d_output_tex(nullptr)
	, _d3d_output_dsv(nullptr)
	, _d3d_output_rtv(nullptr)
	, _show_debuglog(true)
	, _show_viewport(true)
	, _show_imgui_demo(false)
	, _show_implot_demo(false)
	, _show_entity_editor(false)
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

	// Create the IO first as our logging depends on creating the right folder
	_platform_io = IO::create();
	IO::set(_platform_io);

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

	// create the game engine object, exit if failure
	ASSERT(GameEngine::instance());

	// Create DirectX rendering factory
	create_factories();

	Graphics::init(_d3d_device);
	TextureResource::initialise_default();

	// Setup our default overlays
	_overlay_manager = std::make_shared<OverlayManager>();
	_metrics_overlay = new MetricsOverlay();

	_overlay_manager->register_overlay(_metrics_overlay);
	_overlay_manager->register_overlay(new RTTIDebugOverlay());
	_overlay_manager->register_overlay(new ImGuiDemoOverlay());
	_overlay_manager->register_overlay(new ImGuiAboutOverlay());



	// set the instance member variable of the game engine
	this->_hinstance = hInstance;

	// Initialize enkiTS
	s_TaskScheduler = Tasks::get_scheduler();
	Tasks::get_scheduler()->Initialize(max_task_threads);

	struct InitTask : enki::IPinnedTask {
		InitTask(uint32_t threadNum) :
				IPinnedTask(threadNum) {}

		void Execute() override {
			SUCCEEDED(::CoInitialize(NULL));
		}
	};

	std::vector<std::unique_ptr<InitTask>> tasks;
	for(uint32_t i = 0; i < s_TaskScheduler->GetNumTaskThreads(); ++i) {
		tasks.push_back(std::make_unique<InitTask>( i));
		s_TaskScheduler->AddPinnedTask(tasks[i].get());
	}
	s_TaskScheduler->RunPinnedTasks();
	s_TaskScheduler->WaitforAll();

	//Initialize the high precision timers
	_game_timer = make_unique<PrecisionTimer>();
	_game_timer->Reset();

	_input_manager = make_unique<InputManager>();
	_input_manager->Initialize();

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

	LOG_INFO(System, "Initialising worlds...");
	{
		_world = std::make_shared<framework::World>();
		_world->init();

		_render_world = std::make_shared<RenderWorld>();
		_render_world->init();

		_cb_global = ConstantBuffer::create(_d3d_device, sizeof(GlobalDataCB), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);
	}
	LOG_INFO(System, "Finished initialising worlds.");

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

	// Initialize the Graphics Engine
	d3d_init();

	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(get_window());
	ImGui_ImplDX11_Init(_d3d_device, _d3d_device_ctx);
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

	std::vector<ComPtr<ID3D11Query>> gpuTimings[2];
	gpuTimings[0].resize(3);
	gpuTimings[1].resize(3);

	D3D11_QUERY_DESC desc{};
	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	GetD3DDevice()->CreateQuery(&desc, gpuTimings[0][0].GetAddressOf());
	GetD3DDevice()->CreateQuery(&desc, gpuTimings[1][0].GetAddressOf());

	desc.Query = D3D11_QUERY_TIMESTAMP;
	GetD3DDevice()->CreateQuery(&desc, gpuTimings[0][1].GetAddressOf());
	GetD3DDevice()->CreateQuery(&desc, gpuTimings[0][2].GetAddressOf());
	GetD3DDevice()->CreateQuery(&desc, gpuTimings[1][1].GetAddressOf());
	GetD3DDevice()->CreateQuery(&desc, gpuTimings[1][2].GetAddressOf());

	// get time and make sure GameTick is fired before GamePaint
	double previous = _game_timer->GetGameTime() - _physics_timestep;
	double lag = 0; // keep left over time
	_running = true;
	while (_running) {


		// Process all window messages
		MSG msg{};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Running might have been updated by the windows message loop. Handle this here.
		if (!_running) {
			break;
		}

		{
			++_frame_cnt;

			{
				double current = _game_timer->GetGameTime();
				double elapsed = current - previous; // calc timedifference
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::FrameTime, (float)(elapsed * 1000.0f));
				if (elapsed > 0.25) elapsed = 0.25; //prevent jumps in time when break point or sleeping
				previous = current;  // reset
				lag += elapsed;

				Timer t{};
				t.Start();
				while (lag >= _physics_timestep)
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
					lag -= _physics_timestep;

					// Input manager update takes care of swapping the state
					_input_manager->Update();
				}
				t.Stop();
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::GameUpdateCPU, (float)t.GetTimeInMS());
			}

			if (_recreate_swapchain)
			{
				LOG_VERBOSE(Graphics,"Recreating swapchain. New size: %dx%d\n", (uint32_t)_window_width, (uint32_t)_window_height);


				this->resize_swapchain(_window_width, _window_height);
				_recreate_swapchain = false;
			}

			// Recreating the game viewport texture needs to happen before running IMGUI and the actual rendering
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			build_ui();
			//{
				ImVec2 game_width = { get_width() / 2.0f, get_height() / 2.0f };
				ImGui::SetNextWindowSize(game_width, ImGuiCond_FirstUseEver);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
				ImGui::PopStyleVar(1);

			//	if (_frame_cnt >= 2)
			//	{
			//		_game->debug_ui();
			//	}
				_overlay_manager->render_overlay();
			//}
			ImGui::EndFrame();
			ImGui::UpdatePlatformWindows();
			ImGui::Render();

			// Get gpu data 
			size_t idx = _frame_cnt % 2;
			if (_frame_cnt > 2)
			{
				D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestampDisjoint;
				UINT64 start;
				UINT64 end;
				while (S_OK != _d3d_device_ctx->GetData(gpuTimings[idx][0].Get(), &timestampDisjoint, sizeof(timestampDisjoint), 0)) {}
				while (S_OK != _d3d_device_ctx->GetData(gpuTimings[idx][1].Get(), &start, sizeof(UINT64), 0)) {}
				while (S_OK != _d3d_device_ctx->GetData(gpuTimings[idx][2].Get(), &end, sizeof(UINT64), 0)) {}

				double diff = (double)(end - start) / (double)timestampDisjoint.Frequency;
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::RenderGPU, (float)(diff * 1000.0));
			}
		}

		ResourceLoader::instance()->update();

		GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Frame");
		size_t idx = _frame_cnt % 2;

		_d3d_device_ctx->Begin(gpuTimings[idx][0].Get());
		_d3d_device_ctx->End(gpuTimings[idx][1].Get());

		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<float>(this->get_viewport_size().x);
		vp.Height = static_cast<float>(this->get_viewport_size().y);
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		_d3d_device_ctx->RSSetViewports(1, &vp);


		if (_render_world->get_view_camera()) {
			//_render_world->get_view_camera()->set_aspect((f32)vp.Width / (f32)vp.Height);
			//_render_world->get_view_camera()->update();
		}


		// Render 3D before 2D
		if(_engine_settings.d3d_use)
		{
			GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Render");

			FLOAT color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
			_d3d_device_ctx->ClearRenderTargetView(_d3d_output_rtv, color);
			_d3d_device_ctx->ClearDepthStencilView(_d3d_output_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

			{
				GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Shadows");

				if(!_shadow_map) {
				
					auto res_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32_TYPELESS, 2048, 2048);
					res_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
					res_desc.MipLevels = 1;
					if (FAILED(_d3d_device->CreateTexture2D(&res_desc, NULL, _shadow_map.ReleaseAndGetAddressOf())))
					{
						ASSERTMSG(false, "Failed to create the shadowmap texture");
					}

					auto view_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(_shadow_map.Get(), D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT);
					if (FAILED(_d3d_device->CreateDepthStencilView(_shadow_map.Get(), &view_desc, _shadow_map_dsv.ReleaseAndGetAddressOf())))
					{
						ASSERTMSG(false, "Failed to create the shadowmap DSV");
					}

					auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(_shadow_map.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT);
					if (FAILED(_d3d_device->CreateShaderResourceView(_shadow_map.Get(), &srv_desc, _shadow_map_srv.ReleaseAndGetAddressOf()))) {
						ASSERTMSG(false, "Failed to create the shadowmap SRV");
					}
				}
				_d3d_device_ctx->OMSetRenderTargets(0, nullptr, _shadow_map_dsv.Get());
				_d3d_device_ctx->ClearDepthStencilView(_shadow_map_dsv.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0);

				shared_ptr<RenderWorldLight> light =  _render_world->get_light(0);
				if(light->get_casts_shadow()) {
					ViewParams params{};
					params.view = light->get_view();
					params.proj = light->get_proj();
					params.view_direction = light->get_view_direction().xyz;
					params.pass = RenderPass::Shadow;
					params.viewport = CD3D11_VIEWPORT(0.0f, 0.0f, 2048, 2048);
					render_world(params);
				}
			}

			// Adding the following 2  lines introduces uncontrolled flickering!
			_render_world->get_view_camera()->set_aspect((f32)m_ViewportWidth / (f32)m_ViewportHeight);
			_render_world->get_view_camera()->update();

			_d3d_device_ctx->OMSetRenderTargets(0, NULL, _d3d_output_dsv);
			//render_view(RenderPass::ZPrePass);

			_d3d_device_ctx->OMSetRenderTargets(1, &_d3d_output_rtv, _d3d_output_dsv);
			render_view(RenderPass::Opaque);
		}

		// Render Direct2D to the swapchain
		if(_engine_settings.d2d_use)
		{
			#if FEATURE_D2D
			d2d_render();
			#else
			LOG_ERROR(System, "Trying to use D2D but the build isn't compiled with D2D enabled!");
			DebugBreak();
			#endif
		}

		// Resolve msaa to non msaa for imgui render
		_d3d_device_ctx->ResolveSubresource(_d3d_non_msaa_output_tex, 0, _d3d_output_tex, 0, _swapchain_format);

		// Render main viewport ImGui
		{
			GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"ImGui");
			_d3d_device_ctx->OMSetRenderTargets(1, &_d3d_backbuffer_view, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}


		// Present
		GPU_MARKER(_d3d_user_defined_annotation, L"DrawEnd");
		u32 flags = 0;
		if(!_vsync_enabled) {
			flags |= DXGI_PRESENT_ALLOW_TEARING;
		}
		_dxgi_swapchain->Present(_vsync_enabled ? 1 : 0, flags);

		_d3d_device_ctx->End(gpuTimings[idx][2].Get());
		_d3d_device_ctx->End(gpuTimings[idx][0].Get());

		// Render all other imgui windows  
		ImGui::RenderPlatformWindowsDefault();
	}

	// Make sure all tasks have finished before shutting down
	Tasks::get_scheduler()->WaitforAllAndShutdown();

	// User defined code for exiting the game
	_game->end();
	_game.reset();

	ResourceLoader::instance()->unload_all();
	ResourceLoader::Shutdown();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	// Teardown graphics resources and windows procedures
	{
		_default_font.reset();

		// Deinit our global graphics API before killing the game engine API
		Graphics::deinit();

		// deinit the engine graphics layer
		d3d_deinit();

		::CoUninitialize();
	}

	return 0;
}

#if FEATURE_D2D
void GameEngine::d2d_render()
{
	GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Game2D");

	graphics::D2DRenderContext context{ _d2d_factory, _d2d_rt, _color_brush, _default_font.get() };
	context.begin_paint();
	_d2d_ctx = &context;

	auto size = this->get_viewport_size();

	_can_paint = true;
	// make sure tvp.Heighthe view matrix is taken in account
	context.set_world_matrix(float3x3::identity());
	_game->paint(context);

	// draw Box2D debug rendering
	// http://www.iforce2d.net/b2dtut/debug-draw
	if (_debug_physics_rendering)
	{
		// dimming rect in screenspace
		context.set_world_matrix(float3x3::identity());
		float3x3 matView = context.get_view_matrix();
		context.set_view_matrix(float3x3::identity());
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
	if (!result) PostMessage(GameEngine::get_window(), WM_DESTROY, 0, 0);
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
	if (!RegisterClassEx(&wndclass)) return false;
	return true;
}

bool GameEngine::open_window(int iCmdShow)
{
	// Calculate the window size and position based upon the game size
	DWORD windowStyle = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_OVERLAPPEDWINDOW;
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

	if (!_hwindow) return false;

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

	return true;
}

void GameEngine::resize_swapchain(uint32_t width, uint32_t height) {
	LOG_VERBOSE(Graphics, "Resizing swapchain to {}x{}", width, height);
	DXGI_FORMAT swapchain_format = DXGI_FORMAT_B8G8R8A8_UNORM;
	_swapchain_format = swapchain_format;

	// Create MSAA render target that resolves to non-msaa swapchain
	DXGI_SAMPLE_DESC aa_desc{};
	switch (_engine_settings.d3d_msaa_mode) {
		case MSAAMode::Off:
			aa_desc.Count = 1;
			break;
		case MSAAMode::MSAA_2x:
			aa_desc.Count = 2;
			break;
		case MSAAMode::MSAA_4x:
			aa_desc.Count = 4;
			break;
		default:
			break;
	}

	UINT qualityLevels;
	_d3d_device->CheckMultisampleQualityLevels(swapchain_format, _aa_desc.Count, &qualityLevels);
	aa_desc.Quality = (_engine_settings.d3d_msaa_mode != MSAAMode::Off) ? qualityLevels - 1 : 0;

	// Release the textures before re-creating the swapchain
	if (_dxgi_swapchain) {
		helpers::SafeRelease(_d3d_non_msaa_output_tex);
		helpers::SafeRelease(_d3d_non_msaa_output_srv);
		helpers::SafeRelease(_d3d_output_tex);
		helpers::SafeRelease(_d3d_output_rtv);
		helpers::SafeRelease(_d3d_output_srv);

		helpers::SafeRelease(_d3d_output_depth);
		helpers::SafeRelease(_d3d_output_dsv);
		helpers::SafeRelease(_d2d_rt);
		helpers::SafeRelease(_d3d_backbuffer_view);
		helpers::SafeRelease(_d3d_backbuffer_srv);
	}

	// Create the 3D output target
	{
		auto output_desc = CD3D11_TEXTURE2D_DESC(
				swapchain_format,
				get_width(),
				get_height(),
				1,
				1,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
		SUCCEEDED(_d3d_device->CreateTexture2D(&output_desc, nullptr, &_d3d_output_tex));
		SUCCEEDED(_d3d_device->CreateRenderTargetView(_d3d_output_tex, nullptr, &_d3d_output_rtv));
		SUCCEEDED(_d3d_device->CreateShaderResourceView(_d3d_output_tex, nullptr, &_d3d_output_srv));

		// This texture is used to output to a image in imgui
		output_desc.SampleDesc.Count = 1;
		output_desc.SampleDesc.Quality = 0;
		SUCCEEDED(_d3d_device->CreateTexture2D(&output_desc, nullptr, &_d3d_non_msaa_output_tex));
		SUCCEEDED(_d3d_device->CreateShaderResourceView(_d3d_non_msaa_output_tex, nullptr, &_d3d_non_msaa_output_srv));
	}


	// Create the 3D depth target
	auto dsv_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, get_width(), get_height(), 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
	SUCCEEDED(_d3d_device->CreateTexture2D(&dsv_desc, nullptr, &_d3d_output_depth));
	SUCCEEDED(_d3d_device->CreateDepthStencilView(_d3d_output_depth, NULL, &_d3d_output_dsv));

	set_debug_name(_d3d_output_tex, "Color Output");
	set_debug_name(_d3d_output_depth, "Depth Output");

	// Either create the swapchain or retrieve the existing description
	DXGI_SWAP_CHAIN_DESC desc{};
	if (_dxgi_swapchain) {
		_dxgi_swapchain->GetDesc(&desc);
		_dxgi_swapchain->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, desc.Flags);
	} else {
		desc.BufferDesc.Width = get_width();
		desc.BufferDesc.Height = get_height();
		desc.BufferDesc.Format = swapchain_format;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		desc.OutputWindow = get_window();
		if (_game_settings.m_FullscreenMode == GameSettings::FullScreenMode::Windowed || _game_settings.m_FullscreenMode == GameSettings::FullScreenMode::BorderlessWindowed) {
			desc.Windowed = TRUE;
		} else {
			desc.Windowed = false;
		}
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		SUCCEEDED(_dxgi_factory->CreateSwapChain(_d3d_device, &desc, &_dxgi_swapchain));

		set_debug_name(_dxgi_swapchain, "DXGISwapchain");
		set_debug_name(_dxgi_factory, "DXGIFactory");
	}

	// Recreate the views
	ComPtr<ID3D11Texture2D> backBuffer;
	_dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	assert(backBuffer);
	SUCCEEDED(_d3d_device->CreateRenderTargetView(backBuffer.Get(), NULL, &_d3d_backbuffer_view));
	SUCCEEDED(_d3d_device->CreateShaderResourceView(backBuffer.Get(), NULL, &_d3d_backbuffer_srv));
	set_debug_name(backBuffer.Get(), "Swapchain::Output");

	// Create the D2D target for 2D rendering
	UINT dpi = GetDpiForWindow(get_window());
	D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(swapchain_format, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);

	ComPtr<IDXGISurface> surface;
	_d3d_output_tex->QueryInterface(surface.GetAddressOf());

	if(_d2d_factory) {
		SUCCEEDED(_d2d_factory->CreateDxgiSurfaceRenderTarget(surface.Get(), rtp, &_d2d_rt));
		set_debug_name(surface.Get(), "[D2D] Output");

		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
	}
}

void GameEngine::quit_game()
{
	this->_running = false;
}


void GameEngine::enable_aa(bool isEnabled)
{
	_d2d_aa_mode = isEnabled ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	#if FEATURE_D2D
	if (_d2d_rt) {
		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
	}
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
	return { (float)m_ViewportWidth, (float)m_ViewportHeight };
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

float2 GameEngine::get_mouse_pos_in_window() const {
	RECT rect;
	if(GetWindowRect(get_window(), &rect)) {
		return float2{ (float)_input_manager->get_mouse_position().x, (float)_input_manager->get_mouse_position().y } + float2(rect.left, rect.top);
	}
	return {};
}

float2 GameEngine::get_mouse_pos_in_viewport() const
{
	return float2{ (float)_input_manager->get_mouse_position().x, (float)_input_manager->get_mouse_position().y } - m_ViewportPos;
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
}

void GameEngine::set_height(int iHeight)
{
	_window_height = iHeight;
}

void GameEngine::set_physics_step(bool bEnabled)
{
	_physics_step_enabled = bEnabled;
}

void GameEngine::set_sleep(bool bSleep)
{
	if (_game_timer == nullptr)
		return;

	_should_sleep = bSleep;
	if (bSleep)
	{
		_game_timer->Stop();
	}
	else
	{
		_game_timer->Start();
	}
}

void GameEngine::enable_vsync(bool bEnable)
{
	_vsync_enabled = bEnable;
}

void GameEngine::apply_settings(GameSettings &game_settings)
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


std::shared_ptr<OverlayManager> const& GameEngine::get_overlay_manager() const {
	return _overlay_manager;
}

// Input methods
bool GameEngine::is_key_down(int key) const
{
	return _input_manager->is_key_down(key);
}

bool GameEngine::is_key_pressed(int key) const
{
	return _input_manager->is_key_pressed(key);
}

bool GameEngine::is_key_released(int key) const
{
	return _input_manager->is_key_released(key);
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

	if (ImGui::GetCurrentContext() != nullptr) {
		bool bWantImGuiCapture = ImGui::GetIO().WantCaptureKeyboard ||
								 ImGui::GetIO().WantCaptureMouse;

		if (bWantImGuiCapture) {
			extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
			if (LRESULT v = ImGui_ImplWin32_WndProcHandler(hWindow, msg, wParam, lParam); v != 0) {
				return v;
			}
		}
	}


	// Route Windows messages to game engine member functions
	switch (msg)
	{
	case WM_CREATE:
		// Set the game window 
		this->_hwindow = hWindow;
		return 0;
	case WM_SYSCOMMAND:	// trapping this message prevents a freeze after the ALT key is released
		if (wParam == SC_KEYMENU) return 0;			// see win32 API : WM_KEYDOWN
		else break;

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
		if (wParam == VK_F9) {
			_overlay_manager->set_visible(!_overlay_manager->get_visible());
		}
		break;
	}

	bool handled = false;
	handled |= _input_manager->handle_events(msg, wParam, lParam);
	if (handled)
		return 0;

	return DefWindowProc(hWindow, msg, wParam, lParam);
}

// Create resources which are not bound
// to any device. Their lifetime effectively extends for the
// duration of the app. These resources include the Direct2D and
// DirectWrite factories,  and a DirectWrite Text Format object
// (used for identifying particular font characteristics).
void GameEngine::create_factories()
{
	// Create Direct3D 11 factory
	{
		ENSURE_HR(CreateDXGIFactory(IID_PPV_ARGS(&_dxgi_factory)));
		helpers::SetDebugObjectName(_dxgi_factory, "Main DXGI Factory");

		// Define the ordering of feature levels that Direct3D attempts to create.
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
		};
		D3D_FEATURE_LEVEL featureLevel;

		uint32_t creation_flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		bool debug_layer = cli::has_arg(_command_line, "-enable-d3d-debug");
		if(debug_layer) {
			creation_flag |= D3D11_CREATE_DEVICE_DEBUG;
		}
	#if defined(_DEBUG)
		else {
			creation_flag |= D3D11_CREATE_DEVICE_DEBUG;
			debug_layer = true;
		}
	#endif

		ENSURE_HR(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creation_flag, featureLevels, UINT(std::size(featureLevels)), D3D11_SDK_VERSION, &_d3d_device, &featureLevel, &_d3d_device_ctx));

		if (debug_layer) {
			bool do_breaks = cli::has_arg(_command_line, "-d3d-break");
			ComPtr<ID3D11InfoQueue> info_queue;
			_d3d_device->QueryInterface(IID_PPV_ARGS(&info_queue));
			if (info_queue) {
				if (do_breaks) {
					info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
					info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
				}

				D3D11_INFO_QUEUE_FILTER f{};
				f.DenyList.NumSeverities = 1;
				D3D11_MESSAGE_SEVERITY severities[1] = {
					//D3D11_MESSAGE_SEVERITY_WARNING
				};
				f.DenyList.pSeverityList = severities;
				info_queue->AddStorageFilterEntries(&f);
			}
		}
	}

#if FEATURE_D2D
	d2d_create_factory();
#endif

	WIC_create_factory();
	write_create_factory();

	_d3d_device_ctx->QueryInterface(IID_PPV_ARGS(&_d3d_user_defined_annotation));
}

#if FEATURE_D2D
void GameEngine::d2d_create_factory()
{
	if (_engine_settings.d2d_use) {
		HRESULT hr;
		// Create a Direct2D factory.
		ID2D1Factory* localD2DFactoryPtr = nullptr;
		if (!_d2d_factory)
		{
			D2D1_FACTORY_OPTIONS options;
			options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
			hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,options, &localD2DFactoryPtr);
			if (FAILED(hr))
			{
				FAILMSG("Create D2D Factory Failed");
				exit(-1);
			}
			_d2d_factory = localD2DFactoryPtr;
		}
	}
}
#endif

void GameEngine::WIC_create_factory()
{
	HRESULT hr;
	// Create a WIC factory if it does not exists
	IWICImagingFactory* localWICFactoryPtr = nullptr;
	if (!_wic_factory)
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&localWICFactoryPtr));
		if (FAILED(hr))
		{
			FAILMSG("Create WIC Factory Failed");
			exit(-1);
		}
		_wic_factory = localWICFactoryPtr;
	}
}

void GameEngine::write_create_factory()
{
	HRESULT hr;
	// Create a DirectWrite factory.
	IDWriteFactory* localDWriteFactoryPtr = nullptr;
	if (!_dwrite_factory)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(localDWriteFactoryPtr), reinterpret_cast<IUnknown **>(&localDWriteFactoryPtr));
		if (FAILED(hr))
		{
			FAILMSG("Create WRITE Factory Failed");
			exit(-1);
		}
		_dwrite_factory = localDWriteFactoryPtr;
	}
}

//
//  This method creates resources which are bound to a particular
//  Direct3D device. It's all centralized here, in case the resources
//  need to be recreated in case of Direct3D device loss (eg. display
//  change, remoting, removal of video card, etc).
//
void GameEngine::d3d_init()
{
	resize_swapchain(get_width(), get_height());
	m_ViewportWidth = get_width();
	m_ViewportHeight = get_height();
	m_ViewportPos = { 0, 0 };

	if(_d2d_rt) {
		//set alias mode
		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);

		// Create a brush.
		_d2d_rt->CreateSolidColorBrush((D2D1::ColorF) D2D1::ColorF::Black, &_color_brush);
	}

	//Create a Font
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

	using helpers::SafeRelease;

	SafeRelease(_color_brush);
	SafeRelease(_d2d_rt);
	SafeRelease(_d3d_backbuffer_view);

	SafeRelease(_d3d_output_tex);
	SafeRelease(_d3d_output_rtv);
	SafeRelease(_d3d_output_srv);
	SafeRelease(_d3d_output_depth);
	SafeRelease(_d3d_output_dsv);
	SafeRelease(_d3d_backbuffer_srv);
	SafeRelease(_d3d_user_defined_annotation);

	SafeRelease(_dxgi_swapchain);

	SafeRelease(_d3d_device_ctx);
	SafeRelease(_d3d_device);
	SafeRelease(_dwrite_factory);
	SafeRelease(_wic_factory);
	SafeRelease(_d2d_factory);
	SafeRelease(_dxgi_factory);
}


// Box2D overloads
void GameEngine::BeginContact(b2Contact* contactPtr)
{
    b2Fixture * fixAPtr = contactPtr->GetFixtureA();
    b2Fixture * fixBPtr = contactPtr->GetFixtureB();

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
    b2Fixture * fixAPtr = contactPtr->GetFixtureA();
    b2Fixture * fixBPtr = contactPtr->GetFixtureB();

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
	b2Fixture * fixAPtr = contactPtr->GetFixtureA();
	b2Fixture * fixBPtr = contactPtr->GetFixtureB();

	ImpulseData impulseData;	
	impulseData.contactListenerAPtr = fixAPtr->GetBody()->GetUserData();
	impulseData.contactListenerBPtr = fixBPtr->GetBody()->GetUserData();	
	impulseData.actAPtr = fixAPtr->GetUserData();
	impulseData.actBPtr = fixBPtr->GetUserData();
	
	// normalImpulses[1] seems to be always 0, add them up
	if (impulsePtr->count>0)impulseData.impulseA = impulsePtr->normalImpulses[0];
	if (impulsePtr->count>1)impulseData.impulseB = impulsePtr->normalImpulses[1];

	double sum = impulseData.impulseA + impulseData.impulseB;
	impulseData.impulseA = impulseData.impulseB = sum;

	if(sum > 0.00001) _b2d_impulse_data.push_back(impulseData);
}

void GameEngine::CallListeners()
{
	// begin contact
	for (size_t i = 0; i < _b2d_begin_contact_data.size(); i++)
	{
		ContactListener * contactListenerPtr = reinterpret_cast<ContactListener *>(_b2d_begin_contact_data[i].contactListenerPtr);
		contactListenerPtr->BeginContact(
			reinterpret_cast<PhysicsActor *>(_b2d_begin_contact_data[i].actThisPtr),
			reinterpret_cast<PhysicsActor *>(_b2d_begin_contact_data[i].actOtherPtr)
		);
	}
	_b2d_begin_contact_data.clear();

	// end contact
	for (size_t i = 0; i < _b2d_end_contact_data.size(); i++)
	{
   
            ContactListener * contactListenerPtr = reinterpret_cast<ContactListener *>(_b2d_end_contact_data[i].contactListenerPtr);
            contactListenerPtr->EndContact(
                reinterpret_cast<PhysicsActor *>(_b2d_end_contact_data[i].actThisPtr),
                reinterpret_cast<PhysicsActor *>(_b2d_end_contact_data[i].actOtherPtr)
                );
		
	}
	_b2d_end_contact_data.clear();

	// impulses
	for (size_t i = 0; i < _b2d_impulse_data.size(); i++)
	{
		ContactListener * contactListenerAPtr = reinterpret_cast<ContactListener *>(_b2d_impulse_data[i].contactListenerAPtr);
		ContactListener * contactListenerBPtr = reinterpret_cast<ContactListener *>(_b2d_impulse_data[i].contactListenerBPtr);
		if (contactListenerAPtr != nullptr) contactListenerAPtr->ContactImpulse(reinterpret_cast<PhysicsActor *>(_b2d_impulse_data[i].actAPtr), _b2d_impulse_data[i].impulseA);
		if (contactListenerBPtr != nullptr) contactListenerBPtr->ContactImpulse(reinterpret_cast<PhysicsActor *>(_b2d_impulse_data[i].actBPtr), _b2d_impulse_data[i].impulseB);
	}
	_b2d_impulse_data.clear();
}


// String helpers for hlsl types
template <>
struct fmt::formatter<float4x4> : formatter<string_view> {

	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		auto it = ctx.begin();
		++it;
		return it;
	}

	template <typename FormatContext>
	constexpr auto format(hlslpp::float4x4 const& m, FormatContext& ctx) {
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

void GameEngine::render_view(RenderPass::Value pass) {

	// Setup global constant buffer
	std::shared_ptr<RenderWorldCamera> camera = _render_world->get_view_camera();

	ViewParams params{};
	D3D11_TEXTURE2D_DESC desc;
	_d3d_output_tex->GetDesc(&desc);

	params.proj = camera->get_proj();
	params.view = camera->get_view();

	params.view_direction = camera->get_view_direction().xyz;
	params.pass = pass;

	// Viewport depends on the actual imgui window  
	params.viewport = CD3D11_VIEWPORT(_d3d_output_tex, _d3d_output_rtv);
	params.viewport.Width = (f32)m_ViewportWidth;
	params.viewport.Height = (f32)m_ViewportHeight;

	render_world(params);
}

void GameEngine::render_world(ViewParams const& params) {

#ifdef _DEBUG
	std::string passName = RenderPass::ToString(params.pass);
	GPU_SCOPED_EVENT(_d3d_user_defined_annotation, passName);
#endif

	// Setup some defaults. At the moment these are applied for each pass. However 
	// ideally we would be able to have more detailed logic here to decided based on pass and mesh/material
	{
		DepthStencilState::Value depth_stencil_state = DepthStencilState::GreaterEqual;
		if (params.pass == RenderPass::ZPrePass || params.pass == RenderPass::Shadow) {
			depth_stencil_state = DepthStencilState::GreaterEqual;
		}

		ID3D11SamplerState* samplers[1] = {
			Graphics::GetSamplerState(SamplerState::MinMagMip_Linear).Get(),
		};
		_d3d_device_ctx->PSSetSamplers(0, 1, samplers);

		_d3d_device_ctx->OMSetDepthStencilState(Graphics::GetDepthStencilState(depth_stencil_state).Get(), 0);
		_d3d_device_ctx->RSSetState(Graphics::GetRasterizerState(RasterizerState::CullBack).Get());
		_d3d_device_ctx->OMSetBlendState(Graphics::GetBlendState(BlendState::Default).Get(), NULL, 0xffffffff);
		_d3d_device_ctx->RSSetViewports(1, &params.viewport);
	}



	GlobalDataCB* global = (GlobalDataCB*)_cb_global->map(_d3d_device_ctx);
	global->ambient.ambient = float4(0.02f, 0.02f, 0.02f, 1.0f);

	global->proj = params.proj;
	global->view = params.view;
	global->inv_view = hlslpp::inverse(params.view);
	global->view_direction = float4(params.view_direction.xyz,0.0f);

	RenderWorld::LightCollection const& lights = _render_world->get_lights();
	global->num_lights = std::min<u32>(u32(lights.size()), MAX_LIGHTS);
	for (u32 i = 0; i < global->num_lights; ++i) {
		LightInfo* info = global->lights + i;
		shared_ptr<RenderWorldLight> l = lights[i];
		if (l->is_directional()) {
			info->direction = float4(l->get_view_direction().xyz, 0.0f);
			info->colour = float4(l->get_colour(), 1.0f);
			info->light_space = l->get_vp();
		}
	}

	_cb_global->unmap(_d3d_device_ctx);

	float4x4 vp = hlslpp::mul(params.view, params.proj);

	RenderWorld::InstanceCollection const& instances = _render_world->get_instances();
	for(std::shared_ptr<RenderWorldInstance> const& inst : instances) {

		if (!inst->is_ready())
			continue;

		auto ctx = _d3d_device_ctx;

		RenderWorldInstance::ConstantBufferData* data = (RenderWorldInstance::ConstantBufferData*)inst->_model_cb->map(_d3d_device_ctx);
		data->world = inst->_transform;
		data->wv = hlslpp::mul(data->world, params.view);
		data->wvp = hlslpp::mul(data->world, vp);
		inst->_model_cb->unmap(ctx);

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetIndexBuffer(inst->_mesh->_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		UINT strides = { sizeof(ModelResource::VertexType) };
		UINT offsets = { 0 };
		ctx->IASetVertexBuffers(0, 1, inst->_mesh->_vert_buffer.GetAddressOf(), &strides, &offsets);

		ID3D11Buffer* buffers[2] = { 
			_cb_global->Get(),
			inst->_model_cb->Get()
		};
		ctx->VSSetConstantBuffers(0, 2, buffers);
		ctx->PSSetConstantBuffers(0, 2, buffers);


		for (Mesh const& m : inst->_mesh->_meshes) {
			std::shared_ptr<MaterialResource> res = inst->_mesh->_materials[m.materialID];

			Material* material = res->get();
			if (material->is_double_sided())
			{
				ctx->RSSetState(Graphics::GetRasterizerState(RasterizerState::CullNone).Get());
			} 
			else 
			{
				ctx->RSSetState(Graphics::GetRasterizerState(RasterizerState::CullBack).Get());
			}

			material->apply();
			if(params.pass != RenderPass::Opaque) {
				_d3d_device_ctx->PSSetShader(nullptr, nullptr, 0);
			}

			if(params.pass == RenderPass::Opaque) {
				ID3D11ShaderResourceView* views[] ={_shadow_map_srv.Get()};
				_d3d_device_ctx->PSSetShaderResources(3, 1, views);
			}

			ctx->DrawIndexed((UINT)m.indexCount, (UINT)m.firstIndex, (INT)m.firstVertex);
		}

		if (params.pass == RenderPass::Opaque) {
			// unbind shader resource
			ID3D11ShaderResourceView* views[] = { nullptr };
			_d3d_device_ctx->PSSetShaderResources(3, 1, views);
		}
	}

}


void GameEngine::build_ui() {


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

	if (_show_implot_demo) {
		ImPlot::ShowDemoWindow(&_show_implot_demo);
	}
}


void GameEngine::build_debug_log() {
	if (!_show_debuglog)
		return;

	if (ImGui::Begin("Output Log", &_show_debuglog)) {
		static bool s_scroll_to_bottom = true;
		static bool s_shorten_file = true;
		static char s_filter[256] = {};
		ImGui::InputText("Filter", s_filter, 512);
		ImGui::SameLine();
		ImGui::Checkbox("Scroll To Bottom", &s_scroll_to_bottom);

		//ImGui::BeginTable("LogData", 3);
		ImGui::BeginChild("123");
		auto const& buffer = Logger::instance()->GetBuffer();
		for (auto it = buffer.begin(); it != buffer.end(); ++it) {
			LogEntry const* entry = *it;
			if (s_filter[0] != '\0') {
				std::string msg = entry->to_message();
				bool bPassed = (msg.find(s_filter) != std::string::npos);
				if (!bPassed)
					continue;
			}

			switch (entry->_severity) {
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
			if (s_shorten_file && entry->_file) {
				auto path = std::filesystem::path(filename);
				filename = fmt::format("{}", path.filename().string());
			}
			ImGui::Text("[%s(%d)][%s][%s] %s", filename.c_str(), it->_line, logging::to_string(it->_category), logging::to_string(it->_severity), it->_message.c_str());
			ImGui::PopStyleColor();
		}
		if (Logger::instance()->_hasNewMessages && s_scroll_to_bottom) {
			Logger::instance()->_hasNewMessages = false;
			ImGui::SetScrollHereY();
		}
		//ImGui::EndTable();
		ImGui::EndChild();
	}
	ImGui::End();
}

void GameEngine::build_viewport() {
	if (!_show_viewport)
		return;

	if (ImGui::Begin("Viewport", &_show_viewport)) {
		static ImVec2 s_vp_size = ImGui::GetContentRegionAvail();
		ImVec2 current_size = ImGui::GetContentRegionAvail();

		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		{
			auto min = vMin;
			auto max = vMax;
			min.x += ImGui::GetWindowPos().x;
			min.y += ImGui::GetWindowPos().y;
			max.x += ImGui::GetWindowPos().x;
			max.y += ImGui::GetWindowPos().y;

			m_ViewportPos.x = min.x;
			m_ViewportPos.y = min.y;
			ImGui::GetForegroundDrawList()->AddRect(min, max, IM_COL32(255, 255, 0, 255));
		}

		if (s_vp_size.x != current_size.x || s_vp_size.y != current_size.y) {
			LOG_VERBOSE(Graphics, "Viewport resize detected! From {}x{} to {}x{}", current_size.x, current_size.y, s_vp_size.x, s_vp_size.y);

			// Update the viewport sizes
			s_vp_size = current_size;
			m_ViewportWidth = (u32)s_vp_size.x;
			m_ViewportHeight = (u32)s_vp_size.y;


		}

		// Draw the actual scene image
		ImVec2 max_uv = {};
		max_uv.x = s_vp_size.x / get_width();
		max_uv.y = s_vp_size.y / get_height();

		ImGui::Image(_d3d_non_msaa_output_srv, s_vp_size, ImVec2(0, 0), max_uv);

		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(m_ViewportPos.x, m_ViewportPos.y, float1(m_ViewportWidth), float1(m_ViewportHeight));

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

void GameEngine::build_menubar() {
	if (ImGui::BeginMenuBar()) {
		if (_build_menu) {
			_build_menu(BuildMenuOrder::First);
		}

		if (ImGui::BeginMenu("Simulation")) {
			static bool s_should_simulate = true;
			if (ImGui::MenuItem(s_should_simulate ? "Stop" : "Start", "", nullptr)) {
				s_should_simulate = !s_should_simulate;
				fmt::printf("Toggling Simulation %s.\n", s_should_simulate ? "On" : "Off");

				this->set_sleep(!s_should_simulate);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows")) {
			if (ImGui::MenuItem("[DEMO] ImPlot")) {
				_show_implot_demo = !_show_implot_demo;
			}

			if (ImGui::MenuItem("Debug Log")) {
				_show_debuglog = !_show_debuglog;
			}
			if (ImGui::MenuItem("Viewport")) {
				_show_viewport = !_show_viewport;
			}
			if (ImGui::MenuItem("Entity Editor")) {
				_show_entity_editor = !_show_entity_editor;
				get_overlay_manager()->get_overlay("EntityDebugOverlay")->set_visible(_show_entity_editor);
			}

			if (ImGui::BeginMenu("Overlays")) {
				if (ImGui::IsItemClicked()) {
					get_overlay_manager()->set_visible(!get_overlay_manager()->get_visible());
				}

				for (auto overlay : get_overlay_manager()->get_overlays()) {
					bool enabled = overlay->get_visible();
					if (ImGui::Checkbox(fmt::format("##{}", overlay->get_name()).c_str(), &enabled)) {
						overlay->set_visible(enabled);
					}

					ImGui::SameLine();
					if (ImGui::MenuItem(overlay->get_name())) {
						overlay->set_visible(!overlay->get_visible());
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (_build_menu) {
			_build_menu(BuildMenuOrder::Last);
		}

		ImGui::EndMenuBar();
	}
}

int GameEngine::run_game(HINSTANCE hInstance, cli::CommandLine const& cmdLine, int iCmdShow, unique_ptr<AbstractGame>&& game)
{

#if defined(DEBUG) | defined(_DEBUG)
	//notify user if heap is corrupt
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

	// Enable run-time memory leak check for debug builds.
	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE const h_dll = LoadLibrary(L"dxgidebug.dll");
	assert(h_dll);
	fPtr const dxgi_get_debug_interface = reinterpret_cast<fPtr>(GetProcAddress(h_dll, "DXGIGetDebugInterface"));
	assert(dxgi_get_debug_interface);

	IDXGIDebug* pDXGIDebug;
	dxgi_get_debug_interface(__uuidof(IDXGIDebug), reinterpret_cast<void**>(&pDXGIDebug));
#endif

	int result = 0;

	GameEngine::instance()->set_command_line(cmdLine);
	GameEngine::instance()->set_game(std::move(game));

	result = GameEngine::instance()->run(hInstance, iCmdShow); // run the game engine and return the result

	// Shutdown the game engine to make sure there's no leaks left. 
	GameEngine::Shutdown();

#if defined(DEBUG) | defined(_DEBUG)
	if (pDXGIDebug) {
		pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	}
	helpers::SafeRelease(pDXGIDebug);
#endif

	return result;
}


