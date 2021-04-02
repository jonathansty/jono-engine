#include "engine.pch.h"

#include "GameEngine.h"
#include "ContactListener.h"
#include "AbstractGame.h"

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/RTTIDebugOverlay.h"
#include "debug_overlays/ImGuiOverlays.h"

#include "core/ResourceLoader.h"
#include "core/logging.h"

#include "InputManager.h"
#include "PrecisionTimer.h"
#include "AudioSystem.h"
#include "Font.h"


static constexpr uint32_t max_task_threads = 4;

// Static task scheduler used for loading assets and executing multi threaded work loads
enki::TaskScheduler GameEngine::s_TaskScheduler;

// Thread ID used to identify if we are on the main thread.
std::thread::id GameEngine::s_main_thread;

LRESULT CALLBACK GameEngine::WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route all Windows messages to the game engine
	return GameEngine::instance()->handle_event(hWindow, msg, wParam, lParam);
}


// #TODO: Remove this once full 2D graphics has been refactored into it's own context
using graphics::bitmap_interpolation_mode;

GameEngine::GameEngine() 
	: _hinstance(0)
	, _hwindow(NULL)
	, _icon(0)
	, _small_icon(0) //changed in june 2014, reset to false in dec 2014
	, _window_width(0)
	, _window_height(0)
	, _should_sleep(true)
	, _game(nullptr)
	, _console(NULL)
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
{

	// Seed the random number generator
	srand((unsigned int)(GetTickCount64()));

}

GameEngine::~GameEngine()
{

	//Free the console
	if (_console)
	{
		_console = NULL;
	}

	delete _default_font;
    delete _b2d_contact_filter;

	//Direct2D Device dependent related stuff
	d3d_deinit();

	//Direct2D Device independent related stuff
	_d3d_device_ctx->Release();
	_d3d_device->Release();
	_dxgi_factory->Release();

	_dwrite_factory->Release();
	_wic_factory->Release();
	_d2d_factory->Release();

	CoUninitialize();
}

void GameEngine::set_game(AbstractGame* gamePtr)
{
	_game = gamePtr;
}

void GameEngine::set_title(const string& titleRef)
{
	_title = titleRef;
}


int GameEngine::run(HINSTANCE hInstance, int iCmdShow)
{
	_platform_io = IO::create();

	// NOTE(Jonathan): Make this smarter to allow more flexibility.
	_platform_io->mount("Resources");

	ASSERTMSG(_game, "No game has been setup! Make sure to first create a game instance before launching the engine!");
	_game->configure_engine(this->_engine_settings);

	// Validate engine settings
	ASSERTMSG(!(_engine_settings.d2d_use && (_engine_settings.d3d_use && _engine_settings.d3d_msaa_mode != MSAAMode::Off)), " Currently the engine does not support rendering D2D with MSAA because DrawText does not respond correctly!");

	s_main_thread = std::this_thread::get_id();


	// initialize d2d for WIC
	SUCCEEDED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));

	// Create DirectX rendering factory
	create_factories();

	// Setup our default overlays
	_overlay_manager = std::make_shared<OverlayManager>();
	_metrics_overlay = new MetricsOverlay();

	_overlay_manager->register_overlay(_metrics_overlay);
	_overlay_manager->register_overlay(new RTTIDebugOverlay());
	_overlay_manager->register_overlay(new ImGuiDemoOverlay());
	_overlay_manager->register_overlay(new ImGuiAboutOverlay());


	// create the game engine object, exit if failure
	ASSERT(GameEngine::instance());

	// set the instance member variable of the game engine
	this->_hinstance = hInstance;

	// Initialize enkiTS
	s_TaskScheduler.Initialize(max_task_threads);

	struct InitTask : enki::IPinnedTask {
		InitTask(uint32_t threadNum) :
				IPinnedTask(threadNum) {}

		void Execute() override {
			SUCCEEDED(::CoInitialize(NULL));
		}
	};

	std::vector<std::unique_ptr<InitTask>> tasks;
	for(uint32_t i = 0; i < s_TaskScheduler.GetNumTaskThreads(); ++i) {
		tasks.push_back(std::make_unique<InitTask>( i));
		s_TaskScheduler.AddPinnedTask(tasks[i].get());
	}
	s_TaskScheduler.RunPinnedTasks();
	s_TaskScheduler.WaitforAll();

	//Initialize the high precision timers
	_game_timer = make_unique<PrecisionTimer>();
	_game_timer->Reset();

	_input_manager = make_unique<InputManager>();
	_input_manager->Initialize();

	// Sound system
#ifndef WINDOWS7
	_xaudio_system = make_unique<AudioSystem>();
	_xaudio_system->Initialize();
#endif

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
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(get_window());
	ImGui_ImplDX11_Init(_d3d_device, _d3d_device_ctx);
#pragma region Box2D
	// Initialize Box2D
	// Define the gravity vector.
	b2Vec2 gravity((float)_gravity.x, (float)_gravity.y);

	// Construct a world object, which will hold and simulate the rigid bodies.
	_b2d_world = new b2World(gravity);
    _b2d_contact_filter = new b2ContactFilter();
    
    _b2d_world->SetContactFilter(_b2d_contact_filter);
	//m_Box2DWorldPtr->SetContactListener(m_GamePtr);
	_b2d_world->SetContactListener(this);

	_b2d_debug_renderer.SetFlags(b2Draw::e_shapeBit);
	_b2d_debug_renderer.AppendFlags(b2Draw::e_centerOfMassBit);
	_b2d_debug_renderer.AppendFlags(b2Draw::e_jointBit);
	_b2d_debug_renderer.AppendFlags(b2Draw::e_pairBit);
	_b2d_world->SetDebugDraw(&_b2d_debug_renderer);
#pragma endregion


	// User defined functions for start of the game
	_game->start();

	std::array<ID3D11Query*, 3> gpuTimings[2];
	D3D11_QUERY_DESC desc{};
	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	GetD3DDevice()->CreateQuery(&desc, &gpuTimings[0][0]);
	GetD3DDevice()->CreateQuery(&desc, &gpuTimings[1][0]);

	desc.Query = D3D11_QUERY_TIMESTAMP;
	GetD3DDevice()->CreateQuery(&desc, &gpuTimings[0][1]);
	GetD3DDevice()->CreateQuery(&desc, &gpuTimings[0][2]);
	GetD3DDevice()->CreateQuery(&desc, &gpuTimings[1][1]);
	GetD3DDevice()->CreateQuery(&desc, &gpuTimings[1][2]);

	// get time and make sure GameTick is fired before GamePaint
	double previous = _game_timer->GetGameTime() - _physics_timestep;
	double lag = 0; // keep left over time
	//static double timesum = 0, count = 1;
	_running = true;
	while (_running) {
		// Process all window messages
		MSG msg{};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

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
					// Check the state of keyboard and mouse
					_input_manager->Update();

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
				}
				t.Stop();
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::GameUpdateCPU, (float)t.GetTimeInMS());
			}

			if (_recreate_swapchain)
			{
				logging::logf("Recreating swapchain. New size: %dx%d\n", (uint32_t)_window_width, (uint32_t)_window_height);

				this->resize_swapchain(_window_width, _window_height);
				_recreate_swapchain = false;
			}

			// Recreating the game viewport texture needs to happen before running IMGUI and the actual rendering
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			//build_ui();
			{
				ImVec2 game_width = { get_width() / 2.0f, get_height() / 2.0f };
				ImGui::SetNextWindowSize(game_width, ImGuiCond_FirstUseEver);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
				//ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoDecoration);
				//{
				//	m_ViewportFocused = ImGui::IsWindowFocused();
				//	ImVec2 size = ImGui::GetContentRegionAvail();
				//	if (_game_viewport_size.x != size.x || _game_viewport_size.y != size.y)
				//	{
				//		_recreate_game_texture = true;
				//		_game_viewport_size.x = std::max(size.x, 4.f);
				//		_game_viewport_size.y = std::max(size.y, 4.f);
				//	}
				//	RECT r;
				//	::GetWindowRect(m_hWindow, &r);

				//	float left = r.left; 
				//	float top = r.top; 
				//	_game_viewport_offset = { ImGui::GetWindowPos().x - left, ImGui::GetWindowPos().y - top};

				//	ImGui::GetWindowDrawList()->AddImage(_game_output_srv, ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
				//	ImVec2 actual_size{ ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y };
				//	ImGui::Image(_game_output_srv, actual_size);
				//}
				//ImGui::End();
				ImGui::PopStyleVar(1);

				if (_frame_cnt >= 2)
				{
					_game->debug_ui();
				}
				_overlay_manager->render_overlay();
			}
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
				while (S_OK != _d3d_device_ctx->GetData(gpuTimings[idx][0], &timestampDisjoint, sizeof(timestampDisjoint), 0)) {}
				while (S_OK != _d3d_device_ctx->GetData(gpuTimings[idx][1], &start, sizeof(UINT64), 0)) {}
				while (S_OK != _d3d_device_ctx->GetData(gpuTimings[idx][2], &end, sizeof(UINT64), 0)) {}

				double diff = (double)(end - start) / (double)timestampDisjoint.Frequency;
				_metrics_overlay->UpdateTimer(MetricsOverlay::Timer::RenderGPU, (float)(diff * 1000.0));
			}
		}

		ResourceLoader::instance()->update();

		GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Frame");
		size_t idx = _frame_cnt % 2;

		_d3d_device_ctx->Begin(gpuTimings[idx][0]);
		_d3d_device_ctx->End(gpuTimings[idx][1]);

		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<float>(this->get_viewport_size().x);
		vp.Height = static_cast<float>(this->get_viewport_size().y);
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		_d3d_device_ctx->RSSetViewports(1, &vp);

		FLOAT color[4] = { 0.25f,0.25f,0.25f,1.0f };
		//m_D3DDeviceContextPtr->ClearRenderTargetView(_game_output_rtv, color);
		//m_D3DDeviceContextPtr->ClearDepthStencilView(_game_output_dsv, D3D11_CLEAR_DEPTH, 0.0f, 0);
		//m_D3DDeviceContextPtr->OMSetRenderTargets(1, &_game_output_rtv, _game_output_dsv);

		_d3d_device_ctx->ClearRenderTargetView(_d3d_output_rtv, color);
		_d3d_device_ctx->ClearDepthStencilView(_d3d_output_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
		_d3d_device_ctx->OMSetRenderTargets(1, &_d3d_output_rtv, _d3d_output_dsv);


		// Render 3D before 2D
		if(_engine_settings.d3d_use)
		{
			GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Render3D");
			_game->render_3d();
		}

		// Render Direct2D to the swapchain
		if(_engine_settings.d2d_use)
		{
			d2d_render();
		}

		// Render main viewport ImGui
		{
			GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"ImGui");
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		ComPtr<ID3D11Texture2D> backBuffer;
		_dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		_d3d_device_ctx->ResolveSubresource(backBuffer.Get(), 0, _d3d_output_tex, 0, DXGI_FORMAT_B8G8R8A8_UNORM);

		// Present
		GPU_MARKER(_d3d_user_defined_annotation, L"DrawEnd");
		_dxgi_swapchain->Present(_vsync_enabled ? 1 : 0, 0);



		_d3d_device_ctx->End(gpuTimings[idx][2]);
		_d3d_device_ctx->End(gpuTimings[idx][0]);

		// Render all other imgui windows  
		ImGui::RenderPlatformWindowsDefault();
	}

	// Make sure all tasks have finished before shutting down
	s_TaskScheduler.WaitforAllAndShutdown();

	// User defined code for exiting the game
	_game->end();
	delete _game;

	ResourceLoader::instance()->unload_all();
	ResourceLoader::Shutdown();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	// Box2D
	delete _b2d_world;

	return 0;
}

void GameEngine::d2d_render()
{
	GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Game2D");

	graphics::D2DRenderContext context{ _d2d_factory, _d2d_rt, _color_brush, _default_font };
	context.begin_paint();
	_d2d_ctx = &context;

	auto size = this->get_viewport_size();

	_can_paint = true;
	// make sure the view matrix is taken in account
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
		set_color(COLOR(0, 0, 0, 127));
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

bool GameEngine::register_wnd_class()
{
	WNDCLASSEXA wndclass;

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
	wndclass.lpszClassName = _title.c_str();

	// Register the window class
	if (!RegisterClassExA(&wndclass)) return false;
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

	_hwindow = CreateWindowA(_title.c_str(), _title.c_str(),
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

void GameEngine::resize_swapchain(uint32_t width, uint32_t height)
{

	DXGI_FORMAT swapchain_format = DXGI_FORMAT_B8G8R8A8_UNORM;

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
		_d3d_output_tex->Release();
		_d3d_output_tex = nullptr;

		_d3d_output_rtv->Release();
		_d3d_output_rtv = nullptr;

		_d3d_output_depth->Release();
		_d3d_output_depth = nullptr;

		_d3d_output_dsv->Release();
		_d3d_output_dsv = nullptr;

		// Resize the swapchain
		if (_d3d_backbuffer_view) {
			_d3d_backbuffer_view->Release();
			_d3d_backbuffer_view = nullptr;
		}

		if (_d3d_backbuffer_srv) {
			_d3d_backbuffer_srv->Release();
			_d3d_backbuffer_srv = nullptr;
		}

		if (_d2d_rt) {
			_d2d_rt->Release();
			_d2d_rt = nullptr;
		}
	}

	// Create the 3D output target
	auto output_desc = CD3D11_TEXTURE2D_DESC(
		swapchain_format,
		get_width(), 
		get_height(), 
		1, 
		1,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality );
	SUCCEEDED(_d3d_device->CreateTexture2D(&output_desc, nullptr, &_d3d_output_tex));
	SUCCEEDED(_d3d_device->CreateRenderTargetView(_d3d_output_tex, nullptr, &_d3d_output_rtv));


	// Create the 3D depth target
	auto dsv_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, get_width(), get_height(), 1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, aa_desc.Count, aa_desc.Quality);
	SUCCEEDED(_d3d_device->CreateTexture2D(&dsv_desc, nullptr, &_d3d_output_depth));
	SUCCEEDED(_d3d_device->CreateDepthStencilView(_d3d_output_depth, NULL, &_d3d_output_dsv));


	// Either create the swapchain or retrieve the existing description
	DXGI_SWAP_CHAIN_DESC desc{};
	if(_dxgi_swapchain) {
		_dxgi_swapchain->GetDesc(&desc);
		_dxgi_swapchain->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, desc.Flags);
	}
	else {
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

	set_debug_name(_d3d_output_tex, "Color Output");
	set_debug_name(_d3d_output_depth, "Depth Output");
	set_debug_name(backBuffer.Get(), "Swapchain::Output");

	// Create the D2D target for 2D rendering
	UINT dpi = GetDpiForWindow(get_window());
	D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(swapchain_format, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);

	ComPtr<IDXGISurface> surface;
	_d3d_output_tex->QueryInterface(surface.GetAddressOf());

	SUCCEEDED(_d2d_factory->CreateDxgiSurfaceRenderTarget(surface.Get(), rtp, &_d2d_rt));
	set_debug_name(surface.Get(), "[D2D] Output");

	_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
}

void GameEngine::quit_game()
{
	this->_running = false;
}


void GameEngine::print_string(const string& textRef)
{
	fmt::print(textRef);
	::OutputDebugStringA(textRef.c_str());
}

bool GameEngine::is_paint_allowed() const
{
	if (_d2d_ctx ) return true;
	else
	{
#ifdef _DEBUG
		MessageBoxA(NULL, "Painting from outside the GamePaint()...\n\nYOU SHALL NOT PASS!!!", "GameEngine says NO", MB_OK);
#endif
		return false;
	}
}

void GameEngine::set_color(COLOR color)
{
	_d2d_ctx->set_color(color);
}

COLOR GameEngine::get_color()
{
	return _d2d_ctx->get_color();
}

void GameEngine::set_bitmap_interpolation_mode(bitmap_interpolation_mode mode)
{
	_d2d_ctx->set_bitmap_interpolation_mode(mode);
}

void GameEngine::enable_aa(bool isEnabled)
{
	_d2d_aa_mode = isEnabled ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	if (_d2d_rt) {
		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
	}
}

void GameEngine::enable_physics_debug_rendering(bool isEnabled)
{
	_debug_physics_rendering = isEnabled;
}

void GameEngine::set_font(Font* fontPtr)
{
	_d2d_ctx->set_font(fontPtr);
}

Font* GameEngine::get_font() const
{
	return _d2d_ctx->get_font();
}

void GameEngine::set_default_font()
{
	return _d2d_ctx->set_font(_default_font);
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
	return { (float)_window_width, (float)_window_height };
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

ID3D11Device* GameEngine::GetD3DDevice() const
{
	return _d3d_device;
}

ID3D11DeviceContext* GameEngine::GetD3DDeviceContext() const
{
	return _d3d_device_ctx;
}

ID3D11RenderTargetView* GameEngine::GetD3DBackBufferView() const
{
	return _d3d_backbuffer_view;
}

ID2D1Factory* GameEngine::GetD2DFactory() const
{
	return _d2d_factory;
}

IWICImagingFactory* GameEngine::GetWICImagingFactory() const
{
	return _wic_factory;
}

ID2D1RenderTarget* GameEngine::GetHwndRenderTarget() const
{
	return _d2d_rt;
}

IDWriteFactory* GameEngine::GetDWriteFactory() const
{
	return _dwrite_factory;
}

float2 GameEngine::get_mouse_pos_in_viewport() const
{
	return float2{ (float)_input_manager->GetMousePosition().x, (float)_input_manager->GetMousePosition().y};
}

unique_ptr<AudioSystem> const& GameEngine::GetXAudio() const
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
	return _input_manager->IsKeyboardKeyReleased(key);
}

bool GameEngine::is_mouse_button_down(int button) const
{
	return _input_manager->is_mouse_button_down(button);
}

bool GameEngine::is_mouse_button_pressed(int button) const
{
	return _input_manager->IsMouseButtonPressed(button);
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
	case WM_KEYDOWN: 
		//m_InputPtr->KeyboardKeyPressed(wParam);
		break;
	case WM_KEYUP:
		//m_InputPtr->KeyboardKeyReleased(wParam);
		if (wParam == VK_F9)
		{
			_overlay_manager->set_visible(!_overlay_manager->get_visible());
		}
		break;
	}

	// Handle IMGUI
	extern IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (LRESULT v = ImGui_ImplWin32_WndProcHandler(hWindow, msg, wParam, lParam); v != 0)
	{
		return v;
	}


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
		CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&_dxgi_factory);

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

		SUCCEEDED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, UINT(std::size(featureLevels)), D3D11_SDK_VERSION, &_d3d_device, &featureLevel, &_d3d_device_ctx));

		if (debug_layer) {

			if(cli::has_arg(_command_line, "-d3d-break")) {
				ComPtr<ID3D11InfoQueue> info_queue;
				_d3d_device->QueryInterface(IID_PPV_ARGS(&info_queue));
				if (info_queue) {
					info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
					info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
				}
			}
		}
	}

	d2d_create_factory();
	WIC_create_factory();
	write_create_factory();

	_d3d_device_ctx->QueryInterface(IID_PPV_ARGS(&_d3d_user_defined_annotation));
}

void GameEngine::d2d_create_factory()
{
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
	//set alias mode
	_d2d_rt->SetAntialiasMode(_d2d_aa_mode);

	// Create a brush.
	_d2d_rt->CreateSolidColorBrush((D2D1::ColorF) D2D1::ColorF::Black, &_color_brush);

	//Create a Font
	_default_font = new Font("Consolas", 12);
	_initialized = true;
}

//
//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
//
void GameEngine::d3d_deinit()
{
	_initialized = false;
	if (_color_brush)
	{
		_color_brush->Release();
		_color_brush = nullptr;
	}
	if (_d2d_rt)
	{
		_d2d_rt->Release();
		_d2d_rt = nullptr;
	}

	_d3d_backbuffer_view->Release();
	_d3d_backbuffer_view = nullptr;

	_d3d_output_tex->Release();
	_d3d_output_rtv->Release();
	

	_dxgi_swapchain->Release();
	_dxgi_swapchain = nullptr;
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

void GameEngine::build_ui()
{
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->GetWorkPos());
			ImGui::SetNextWindowSize(viewport->GetWorkSize());
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Simulation"))
			{
				static bool s_should_simulate = true;
				if (ImGui::MenuItem(s_should_simulate ? "Stop" : "Start", "", nullptr))
				{
					s_should_simulate = !s_should_simulate;
					logging::logf("Toggling Simulation %s.\n", s_should_simulate ? "On" : "Off");

					this->set_sleep(!s_should_simulate);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				static bool s_show_debuglog = false;
				static bool s_show_viewport = false;
				static bool s_show_scene_hierarchy = false;
				static bool s_show_properties = false;

				if (ImGui::MenuItem("Debug Log"))
				{
					s_show_debuglog = !s_show_debuglog;
				}
				if (ImGui::MenuItem("Viewport"))
				{
					s_show_viewport = !s_show_viewport;
				}
				if(ImGui::MenuItem("Scene Hierarchy"))
				{
					s_show_scene_hierarchy = !s_show_scene_hierarchy;
				}

				if (ImGui::MenuItem("Properties"))
				{
					s_show_properties = !s_show_properties;
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}
}

int GameEngine::run_game(HINSTANCE hInstance, cli::CommandLine const& cmdLine, int iCmdShow, AbstractGame* game)
{
	//notify user if heap is corrupt
	//HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE const h_dll = LoadLibrary(L"dxgidebug.dll");
	assert(h_dll);
	fPtr const dxgi_get_debug_interface = reinterpret_cast<fPtr>(GetProcAddress(h_dll, "DXGIGetDebugInterface"));
	assert(dxgi_get_debug_interface);

	IDXGIDebug* pDXGIDebug;
	dxgi_get_debug_interface(__uuidof(IDXGIDebug), reinterpret_cast<void**>(&pDXGIDebug));
#endif

	int result = 0;
	// Apply the command line 
	GameEngine::instance()->set_command_line(cmdLine);

	// Apply the game
	GameEngine::instance()->set_game(game);

	// Startup the engine
	result = GameEngine::instance()->run(hInstance, iCmdShow); // run the game engine and return the result

	// Shutdown the game engine to make sure there's no leaks left. 
	GameEngine::Shutdown();

#if defined(DEBUG) | defined(_DEBUG)
	if (pDXGIDebug)
		pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	pDXGIDebug->Release();
#endif

	return result;
}


