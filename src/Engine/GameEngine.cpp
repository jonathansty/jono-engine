#include "stdafx.h"

#include "GameEngine.h"
#include "ContactListener.h"
#include "AbstractGame.h"

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "debug_overlays/MetricsOverlay.h"
#include "debug_overlays/RTTIDebugOverlay.h"
#include "debug_overlays/ImGuiOverlays.h"

#include "Core/ResourceLoader.h"
#include "Core/logging.h"

enki::TaskScheduler GameEngine::s_TaskScheduler;
std::thread::id GameEngine::s_MainThread;

//-----------------------------------------------------------------
// Windows Functions
//-----------------------------------------------------------------
LRESULT CALLBACK GameEngine::WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route all Windows messages to the game engine
	return GameEngine::instance()->handle_event(hWindow, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// OutputDebugString functions
//-----------------------------------------------------------------
void OutputDebugString(const String& text)
{
	OutputDebugString(text.C_str());
}


//-----------------------------------------------------------------
// GameEngine Constructor(s)/Destructor
//-----------------------------------------------------------------
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
	, _game_timer(nullptr)
	, _color_brush(nullptr)
	, _aa_desc({ 1,0 })
	, _hw_bitmap_interpolation_mode(D2D1_BITMAP_INTERPOLATION_MODE_LINEAR)
	, _default_font(nullptr)
	, _user_font(nullptr)
	, _input_manager(nullptr)
	, _xaudio_system(nullptr)
	, _game_settings()
	, _physics_step_enabled(true)
	, _should_quit(false)
	, _is_viewport_focused(false)
	, _recreate_swapchain(false)
	, _recreate_game_texture(false)
	, _debug_physics_rendering(false)
	, _gravity(DOUBLE2(0, 9.81))
{

	// Seed the random number generator
	srand(GetTickCount());

	// Initialize Direct2D system
	CoInitialize(0);
	CreateDeviceIndependentResources();

	_overlay_manager = std::make_shared<OverlayManager>();
	_metrics_overlay = new MetricsOverlay();

	_overlay_manager->register_overlay(_metrics_overlay);
	_overlay_manager->register_overlay(new RTTIDebugOverlay());
	_overlay_manager->register_overlay(new ImGuiDemoOverlay());
	_overlay_manager->register_overlay(new ImGuiAboutOverlay());

	// Start up the keyboard thread
	//m_hKeybThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) ::KeybThreadProc, this, NULL, &m_dKeybThreadID);
}

GameEngine::~GameEngine()
{

	//Free the console
	if (_console)
	{
		_console = NULL;
	}

	delete _input_manager;
	delete _game_timer;
	delete _default_font;
    delete _b2d_contact_filter;

	//Direct2D Device dependent related stuff
	DiscardDeviceResources();

	//Direct2D Device independent related stuff
	_d3d_device_ctx->Release();
	_d3d_device->Release();
	_dxgi_factory->Release();

	_dwrite_factory->Release();
	_wic_factory->Release();
	_d2d_factory->Release();

#ifndef WINDOWS7
	delete _xaudio_system;
#endif

	CoUninitialize();
}

void GameEngine::set_game(AbstractGame* gamePtr)
{
	_game = gamePtr;
}

//-----------------------------------------------------------------
// Game Engine General Methods
//-----------------------------------------------------------------
void GameEngine::set_title(const String& titleRef)
{
	_title = titleRef;
}


int GameEngine::run(HINSTANCE hInstance, int iCmdShow)
{
	s_MainThread = std::this_thread::get_id();

	// Initialize some windows stuff
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))

	// create the game engine object, exit if failure
	assert(GameEngine::instance());

	// set the instance member variable of the game engine
	this->set_instance(hInstance);

	// Initialize enkiTS
	// TODO: Hookup profiler callbacks
	s_TaskScheduler.Initialize();

	//Initialize the high precision timers
	_game_timer = new PrecisionTimer();
	_game_timer->Reset();

	// Inputmanager
	_input_manager = new InputManager();
	_input_manager->Initialize();

	// Sound system
#ifndef WINDOWS7
	_xaudio_system = new AudioSystem();
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
	CreateDeviceResources();

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

	TIMECAPS tc;
	// Set time period to 1 msec, does not garantee this setting ( practical: set to 1 results in 2 as min on one system)
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	timeBeginPeriod(tc.wPeriodMin);

	// Enter the main message loop

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
	for(;;)
	{
		// Process all window messages
		MSG msg{};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (_should_quit)
			break;

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

					//tick GUI -> for blinking caret
					GUITick(_physics_timestep);

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
				size_t prev_idx = (_frame_cnt - 1) % 2;
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

		_d3d_device_ctx->ClearRenderTargetView(_d3d_backbuffer_view, color);
		_d3d_device_ctx->OMSetRenderTargets(1, &_d3d_backbuffer_view, nullptr);


		// Render 3D before 2D
		{
			GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Render3D");
			_game->render_3d();
		}

		// Render Direct2D to the swapchain
		{
			ExecuteDirect2DPaint();
		}

		// Render main viewport ImGui
		{
			GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"ImGui");
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		// Present
		GPU_MARKER(_d3d_user_defined_annotation, L"DrawEnd");
		_dxgi_swapchain->Present(_vsync_enabled ? 1 : 0, 0);



		_d3d_device_ctx->End(gpuTimings[idx][2]);
		_d3d_device_ctx->End(gpuTimings[idx][0]);

		// Render all other imgui windows  
		ImGui::RenderPlatformWindowsDefault();
	}
	// undo the timer setting
	timeEndPeriod(tc.wPeriodMin);

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

void GameEngine::ExecuteDirect2DPaint()
{
	GPU_SCOPED_EVENT(_d3d_user_defined_annotation, L"Game2D");

	d2d_begin_paint();
	auto size = this->get_viewport_size();
	RECT usedClientRect = { 0, 0, (LONG)size.x, (LONG)size.y };

	_can_paint = true;
	// make sure the view matrix is taken in account
	set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
	_game->paint(usedClientRect);

	//Paint the buttons and textboxes
	GUIPaint();

	// draw Box2D debug rendering
	// http://www.iforce2d.net/b2dtut/debug-draw
	if (_debug_physics_rendering)
	{
		// dimming rect in screenspace
		set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
		MATRIX3X2 matView = get_view_matrix();
		set_view_matrix(MATRIX3X2::CreateIdentityMatrix());
		set_color(COLOR(0, 0, 0, 127));
		FillRect(0, 0, get_width(), get_height());
		set_view_matrix(matView);

		_b2d_world->DebugDraw();
	}

	// deactivate all gui objects
	GUIConsumeEvents();

	_can_paint = false;
	bool result = d2d_end_paint();

	// if drawing failed, terminate the game
	if (!result) PostMessage(GameEngine::get_window(), WM_DESTROY, 0, 0);
}

bool GameEngine::register_wnd_class()
{
	WNDCLASSEX wndclass;

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
	wndclass.lpszClassName = _title.C_str();

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

	_hwindow = CreateWindow(_title.C_str(), _title.C_str(),
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

	// Resize the swapchain
	if (_d3d_backbuffer_view) _d3d_backbuffer_view->Release();
	if (_d3d_backbuffer_srv)  _d3d_backbuffer_srv->Release();

	DXGI_SWAP_CHAIN_DESC desc;
	_dxgi_swapchain->GetDesc(&desc);
	_dxgi_swapchain->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, desc.Flags);

	// Recreate the views
	ID3D11Texture2D* backBuffer;
	_dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	assert(backBuffer);
	SUCCEEDED(_d3d_device->CreateRenderTargetView(backBuffer, NULL, &_d3d_backbuffer_view));
	SUCCEEDED(_d3d_device->CreateShaderResourceView(backBuffer, NULL, &_d3d_backbuffer_srv));

	UINT dpi = GetDpiForWindow(get_window());
	D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);

	IDXGISurface* surface;
	backBuffer->QueryInterface(&surface);

	SUCCEEDED(_d2d_factory->CreateDxgiSurfaceRenderTarget(surface, rtp, &_d2d_rt));
	surface->Release();

	backBuffer->Release();
}

void GameEngine::quit_game()
{
	PostMessage(GameEngine::get_window(), WM_DESTROY, 0, 0);
}

void GameEngine::message_box(const String &text) const
{
	if constexpr (sizeof(TCHAR) == 2)	::MessageBoxW(get_window(),(wchar_t*)text.C_str(), (wchar_t*)_title.C_str(), MB_ICONEXCLAMATION | MB_OK);
	else MessageBoxA(get_window(), (char*)text.C_str(), (char*)_title.C_str(), MB_ICONEXCLAMATION | MB_OK);
}

void GameEngine::console_create()
{
	if (_console == NULL && AllocConsole())
	{
		//get the console handle
		_console = GetStdHandle(STD_OUTPUT_HANDLE);
		//set new console title
		SetConsoleTitle((String("Console ") + _title).C_str());
		// STDOUT redirection
		int  conHandle = _open_osfhandle(PtrToLong(_console), 0x4000);
		FILE* fp = _fdopen(conHandle, "w");
		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);
	}
}

void GameEngine::console_set_fore_color(bool red, bool green, bool blue, bool intensity)
{
	//retrieve current color settings
	CONSOLE_SCREEN_BUFFER_INFO buffer = {};
	GetConsoleScreenBufferInfo(_console, &buffer);

	//copy the background color attributes
	WORD wAttributes = buffer.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	//set the fore color attributes
	if (red) wAttributes |= FOREGROUND_RED;
	if (green) wAttributes |= FOREGROUND_GREEN;
	if (blue) wAttributes |= FOREGROUND_BLUE;
	if (intensity) wAttributes |= FOREGROUND_INTENSITY;
	//set the new color attributes to the console
	SetConsoleTextAttribute(_console, wAttributes);
}

void GameEngine::console_set_back_color(bool red, bool green, bool blue, bool intensity)
{
	//retrieve current color settings
	CONSOLE_SCREEN_BUFFER_INFO buffer = {};
	GetConsoleScreenBufferInfo(_console, &buffer);

	//copy the fore color attributes
	WORD wAttributes = buffer.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	//set the back color attributes
	if (red) wAttributes |= BACKGROUND_RED;
	if (green) wAttributes |= BACKGROUND_GREEN;
	if (blue) wAttributes |= BACKGROUND_BLUE;
	if (intensity) wAttributes |= BACKGROUND_INTENSITY;
	//set the new color attributes to the console
	SetConsoleTextAttribute(_console, wAttributes);
}

void GameEngine::console_set_cursor_position(int column, int row)
{
	//cool trick to avoid the use of the lettersX andY on COORD --> those are not compatible with the renaming used in the template classesXandY class templates
	struct coord {
		SHORT column;
		SHORT row;
	};
	union MyUnion
	{
		coord cursorPosition;
		COORD COORD;
	}myUnion;

	myUnion.cursorPosition.column = (SHORT)column;
	myUnion.cursorPosition.row = (SHORT)row;
	SetConsoleCursorPosition(_console, myUnion.COORD);
}

void GameEngine::print_string(const String& textRef)
{
#ifdef _UNICODE
	std::wcout << textRef.C_str() << "\n";
#else
	std::cout << textRef.C_str() << "\n";
#endif
	String copy = textRef + String("\n");
	::OutputDebugString(copy);
}

void GameEngine::print_string(std::string const& msg)
{
	std::cout << msg.c_str() << "\n";
	::OutputDebugStringA(msg.c_str());
}

//void GameEngine::ConsolePrintString(string text)
//{
//#ifdef _UNICODE
//	wstring wstr(text.begin(), text.end());
//	std::wcout << wstr << std::endl;
//#else
//	std::cout << text << std::endl;
//#endif
//}

void GameEngine::print_string(const String& textRef, int column, int row)
{
	console_set_cursor_position(column, row);
	print_string(textRef);
}

//void GameEngine::ConsolePrintString(string text, int column, int row) 
//{
//	ConsoleSetCursorPosition(column,  row);
//	ConsolePrintString(text);
//}

void GameEngine::console_clear() const
{
	system("cls");
}

void GameEngine::set_instance(HINSTANCE hInstance)
{
	_hinstance = hInstance;
}

void GameEngine::set_window(HWND hWindow)
{
	_hwindow = hWindow;
}

bool GameEngine::is_paint_allowed() const
{
	if (_can_paint) return true;
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
	_color_brush->SetColor(D2D1::ColorF((FLOAT)(color.red / 255.0), (FLOAT)(color.green / 255.0), (FLOAT)(color.blue / 255.0), (FLOAT)(color.alpha / 255.0)));
}

COLOR GameEngine::get_color()
{
	D2D1_COLOR_F dColor = _color_brush->GetColor();
	return COLOR((unsigned char)(dColor.r * 255), (unsigned char)(dColor.g * 255), (unsigned char)(dColor.b * 255), (unsigned char)(dColor.a * 255));
}

bool GameEngine::DrawSolidBackground(COLOR backgroundColor)
{
	if (!is_paint_allowed()) return false;

	_d2d_rt->Clear(D2D1::ColorF((FLOAT)(backgroundColor.red / 255.0), (FLOAT)(backgroundColor.green / 255.0), (FLOAT)(backgroundColor.blue / 255.0), (FLOAT)(backgroundColor.alpha)));

	return true;
}

bool GameEngine::DrawLine(int x1, int y1, int x2, int y2)
{
	return DrawLine(DOUBLE2(x1, y1), DOUBLE2(x2, y2), 1.0);
}

bool GameEngine::DrawLine(DOUBLE2 p1, DOUBLE2 p2, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	_d2d_rt->DrawLine(Point2F((FLOAT)p1.x, (FLOAT)p1.y), Point2F((FLOAT)p2.x, (FLOAT)p2.y), _color_brush, (FLOAT)strokeWidth);

	return true;
}

bool GameEngine::DrawPolygon(const std::vector<POINT>& ptsArr, unsigned int count, bool close)
{
	if (!is_paint_allowed()) return false;
	//unsigned int count = ptsArr.size();
	//do not draw an empty polygon
	if (count<2)return false;

	for (unsigned int countLoop = 0; countLoop < count - 1; ++countLoop)
	{
		DrawLine(ptsArr[countLoop].x, ptsArr[countLoop].y, ptsArr[countLoop + 1].x, ptsArr[countLoop + 1].y);
	}
	if (close)
	{
		DrawLine(ptsArr[0].x, ptsArr[0].y, ptsArr[count - 1].x, ptsArr[count - 1].y);
	}

	return true;
}

bool GameEngine::DrawPolygon(const std::vector<DOUBLE2>& ptsArr, unsigned int count, bool close, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	//unsigned int count = ptsArr.size();

	//do not draw an empty polygon
	if (count<2)return false;

	for (unsigned int countLoop = 0; countLoop < count - 1; ++countLoop)
	{
		DrawLine(ptsArr[countLoop], ptsArr[countLoop + 1], strokeWidth);
	}
	if (close)
	{
		DrawLine(ptsArr[0], ptsArr[count - 1], strokeWidth);
	}

	return true;
}

bool GameEngine::FillPolygon(const std::vector<POINT>& ptsArr, unsigned int count)
{
	if (!is_paint_allowed()) return false;
	//unsigned int count = ptsArr.size();

	//do not fill an empty polygon
	if (count<2)return false;

	HRESULT hr;

	// Create path geometry
	ID2D1PathGeometry *geometryPtr;
	hr = _d2d_factory->CreatePathGeometry(&geometryPtr);
	if (FAILED(hr))
	{
		geometryPtr->Release();
		this->message_box(String("Failed to create path geometry"));
		return false;
	}

	// Write to the path geometry using the geometry sink.
	ID2D1GeometrySink* geometrySinkPtr = nullptr;
	hr = geometryPtr->Open(&geometrySinkPtr);
	if (FAILED(hr))
	{
		geometrySinkPtr->Release();
		geometryPtr->Release();
		this->message_box(String("Failed to open path geometry"));
		return false;
	}
	if (SUCCEEDED(hr))
	{
		geometrySinkPtr->BeginFigure(
			D2D1::Point2F((FLOAT)ptsArr[0].x, (FLOAT)ptsArr[0].y),
			D2D1_FIGURE_BEGIN_FILLED
			);

		for (unsigned int i = 0; i<count; ++i)
		{
			geometrySinkPtr->AddLine(D2D1::Point2F((FLOAT)ptsArr[i].x, (FLOAT)ptsArr[i].y));
		}

		geometrySinkPtr->EndFigure(D2D1_FIGURE_END_CLOSED);

		hr = geometrySinkPtr->Close();
		geometrySinkPtr->Release();
	}
	if (SUCCEEDED(hr))
	{
		_d2d_rt->FillGeometry(geometryPtr, _color_brush);
		geometryPtr->Release();
		return true;
	}

	geometryPtr->Release();
	return false;
}

bool GameEngine::FillPolygon(const std::vector<DOUBLE2>& ptsArr, unsigned int count)
{
	if (!is_paint_allowed())return false;
	//unsigned int count = ptsArr.size();

	//do not fill an empty polygon
	if (count<2)return false;

	HRESULT hr;

	// Create path geometry
	ID2D1PathGeometry *geometryPtr;
	hr = _d2d_factory->CreatePathGeometry(&(geometryPtr));
	if (FAILED(hr))
	{
		geometryPtr->Release();
		this->message_box(String("Failed to create path geometry"));
		return false;
	}

	// Write to the path geometry using the geometry sink.
	ID2D1GeometrySink* geometrySinkPtr = nullptr;
	hr = geometryPtr->Open(&geometrySinkPtr);
	if (FAILED(hr))
	{
		geometrySinkPtr->Release();
		geometryPtr->Release();
		this->message_box(String("Failed to open path geometry"));
		return false;
	}

	if (SUCCEEDED(hr))
	{
		geometrySinkPtr->BeginFigure(
			D2D1::Point2F((FLOAT)ptsArr[0].x, (FLOAT)ptsArr[0].y),
			D2D1_FIGURE_BEGIN_FILLED
			);

		for (unsigned int i = 0; i<count; ++i)
		{
			geometrySinkPtr->AddLine(D2D1::Point2F((FLOAT)ptsArr[i].x, (FLOAT)ptsArr[i].y));
		}

		geometrySinkPtr->EndFigure(D2D1_FIGURE_END_CLOSED);

		hr = geometrySinkPtr->Close();
	}

	geometrySinkPtr->Release();

	if (SUCCEEDED(hr))
	{
		_d2d_rt->FillGeometry(geometryPtr, _color_brush);
		geometryPtr->Release();
		return true;
	}

	geometryPtr->Release();
	return false;
}

bool GameEngine::DrawRect(int left, int top, int right, int bottom)
{
	RECT2 rect2(left, top, right, bottom);
	return DrawRect(rect2, 1.0);
}

bool GameEngine::DrawRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, double strokeWidth)
{
	RECT2 rect2(topLeft.x, topLeft.y, rightbottom.x, rightbottom.y);
	return DrawRect(rect2, strokeWidth);
}

bool GameEngine::DrawRect(RECT rect)
{
	RECT2 rect2(rect.left, rect.top, rect.right, rect.bottom);
	return DrawRect(rect2, 1.0);
}

bool GameEngine::DrawRect(RECT2 rect, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) MessageBoxA(NULL, "GameEngine::DrawRect error: invalid dimensions!", "GameEngine says NO", MB_OK);
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	_d2d_rt->DrawRectangle(d2dRect, _color_brush, (FLOAT)strokeWidth);

	return true;
}

bool GameEngine::FillRect(int left, int top, int right, int bottom)
{
	RECT2 rect2(left, top, right, bottom);
	return FillRect(rect2);
}

bool GameEngine::FillRect(DOUBLE2 topLeft, DOUBLE2 rightbottom)
{
	RECT2 rect2(topLeft.x, topLeft.y, rightbottom.x, rightbottom.y);
	return FillRect(rect2);
}

bool GameEngine::FillRect(RECT rect)
{
	RECT2 rect2(rect.left, rect.top, rect.right, rect.bottom);
	return FillRect(rect2);
}

bool GameEngine::FillRect(RECT2 rect)
{
	if (!is_paint_allowed()) return false;
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) MessageBoxA(NULL, "GameEngine::DrawRect error: invalid dimensions!", "GameEngine says NO", MB_OK);

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	_d2d_rt->FillRectangle(d2dRect, _color_brush);

	return true;
}

bool GameEngine::DrawRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->DrawRoundedRectangle(d2dRoundedRect, _color_brush, 1.0);
	return true;
}

bool GameEngine::DrawRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->DrawRoundedRectangle(d2dRoundedRect, _color_brush, (FLOAT)strokeWidth);
	return true;
}

bool GameEngine::DrawRoundedRect(RECT rect, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->DrawRoundedRectangle(d2dRoundedRect, _color_brush, 1.0);
	return true;
}

bool GameEngine::DrawRoundedRect(RECT2 rect, int radiusX, int radiusY, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->DrawRoundedRectangle(d2dRoundedRect, _color_brush, (FLOAT)strokeWidth);
	return true;
}

bool GameEngine::FillRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->FillRoundedRectangle(d2dRoundedRect, _color_brush);
	return true;
}

bool GameEngine::FillRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->FillRoundedRectangle(d2dRoundedRect, _color_brush);
	return true;
}

bool GameEngine::FillRoundedRect(RECT rect, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->FillRoundedRectangle(d2dRoundedRect, _color_brush);
	return true;
}

bool GameEngine::FillRoundedRect(RECT2 rect, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->FillRoundedRectangle(d2dRoundedRect, _color_brush);
	return true;
}

bool GameEngine::DrawEllipse(int centerX, int centerY, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->DrawEllipse(ellipse, _color_brush, 1.0);

	return true;
}

bool GameEngine::DrawEllipse(DOUBLE2 centerPt, double radiusX, double radiusY, double strokeWidth)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->DrawEllipse(ellipse, _color_brush, (FLOAT)strokeWidth);

	return true;
}

bool GameEngine::FillEllipse(int centerX, int centerY, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->FillEllipse(ellipse, _color_brush);

	return true;
}

bool GameEngine::FillEllipse(DOUBLE2 centerPt, double radiusX, double radiusY)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	_d2d_rt->FillEllipse(ellipse, _color_brush);

	return true;
}
bool GameEngine::DrawString(const String& text, RECT boundingRect)
{
	return DrawString(text, boundingRect.left, boundingRect.top, boundingRect.right, boundingRect.bottom);
}

bool GameEngine::DrawString(const String& text, RECT2 boundingRect)
{
	return DrawString(text, (int)boundingRect.left, (int)boundingRect.top, (int)boundingRect.right, (int)boundingRect.bottom);
}

//bool GameEngine::DrawString(string text, RECT boundingRect)
//{
//	return DrawString(text, boundingRect.left, boundingRect.top, boundingRect.right, boundingRect.bottom);
//}
//
//bool GameEngine::DrawString(string text, RECT2 boundingRect)
//{
//	return DrawString(text, (int)boundingRect.left, (int)boundingRect.top, (int)boundingRect.right, (int)boundingRect.bottom);
//}

bool GameEngine::DrawString(const String& text, int left, int top, int right, int bottom)
{
	return DrawString(text, DOUBLE2(left, top), right, bottom);
}

bool GameEngine::DrawString(const String& text, DOUBLE2 topLeft, double right, double bottom)
{
	tstring stext(text.C_str(), text.C_str() + text.Length());
	if (!is_paint_allowed()) return false;

	D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;
	if (right == -1 || bottom == -1) //ignore the right and bottom edge to enable drawing in entire Level
	{
		options = D2D1_DRAW_TEXT_OPTIONS_NONE;
		right = bottom = FLT_MAX;
	}
	D2D1_RECT_F layoutRect = (RectF)((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(right), (FLOAT)(bottom));

	_d2d_rt->DrawText(stext.c_str(), UINT32(stext.length()), _user_font->GetTextFormat(), layoutRect, _color_brush, options);

	return true;
}

//bool GameEngine::DrawString(string text, int left, int top, int right, int bottom)
//{
//	return DrawString(text, DOUBLE2(left, top), right, bottom); 
//}
//
//bool GameEngine::DrawString(string text, DOUBLE2 topLeft, double right, double bottom)
//{
//	if (!CanIPaint()) return false;
//
//	D2D1_SIZE_F dstSize_f = m_RenderTargetPtr->GetSize();
//	D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_CLIP;
//	if (right == -1 || bottom == -1) //ignore the right and bottom edge to enable drawing in entire Level
//	{
//		options = D2D1_DRAW_TEXT_OPTIONS_NONE;
//		right = bottom = FLT_MAX;
//	}
//	D2D1_RECT_F layoutRect = (RectF) ((FLOAT) topLeft.x, (FLOAT) topLeft.y, (FLOAT) (right), (FLOAT) (bottom));
//
//	tstring wText(text.begin(), text.end());
//	m_RenderTargetPtr->DrawText(wText.c_str(), wText.length(), m_UserFontPtr->GetTextFormat(), layoutRect, m_ColorBrushPtr, options);
//
//	return true;
//}

bool GameEngine::DrawBitmap(Bitmap* imagePtr)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return DrawBitmap(imagePtr, DOUBLE2(0, 0), srcRect2);
}

bool GameEngine::DrawBitmap(Bitmap* imagePtr, RECT srcRect)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	return DrawBitmap(imagePtr, DOUBLE2(0, 0), srcRect2);
}

bool GameEngine::DrawBitmap(Bitmap* imagePtr, int x, int y)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return DrawBitmap(imagePtr, DOUBLE2(x, y), srcRect2);
}

bool GameEngine::DrawBitmap(Bitmap* imagePtr, int x, int y, RECT srcRect)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	return DrawBitmap(imagePtr, DOUBLE2(x, y), srcRect2);
}

bool GameEngine::DrawBitmap(Bitmap* imagePtr, DOUBLE2 position)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return DrawBitmap(imagePtr, position, srcRect2);
}

bool GameEngine::DrawBitmap(Bitmap* imagePtr, DOUBLE2 position, RECT2 srcRect)
{
	if (!is_paint_allowed()) return false;
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	//The size and position, in device-independent pixels in the bitmap's coordinate space, of the area within the bitmap to draw.
	D2D1_RECT_F srcRect_f;
	srcRect_f.left = (FLOAT)srcRect.left;
	srcRect_f.right = (FLOAT)srcRect.right;
	srcRect_f.top = (FLOAT)srcRect.top;
	srcRect_f.bottom = (FLOAT)srcRect.bottom;

	//http://msdn.microsoft.com/en-us/library/dd371880(v=VS.85).aspx
	//The size and position, in device-independent pixels in the render target's coordinate space, 
	//of the area to which the bitmap is drawn. If the rectangle is not well-ordered, nothing is drawn, 
	//but the render target does not enter an error state.
	D2D1_RECT_F dstRect_f;
	dstRect_f.left = (FLOAT)position.x;
	dstRect_f.right = dstRect_f.left + (FLOAT)(srcRect.right - srcRect.left);
	dstRect_f.top = (FLOAT)position.y;
	dstRect_f.bottom = dstRect_f.top + (FLOAT)(srcRect.bottom - srcRect.top);

	_d2d_rt->DrawBitmap(imagePtr->GetBitmapPtr(), dstRect_f, (FLOAT)imagePtr->GetOpacity(), _hw_bitmap_interpolation_mode, srcRect_f);

	return true;
}

//bool GameEngine::DrawHitRegion(HitRegion* collisionMeshPtr)
//{
//	if (!CanIPaint()) return false;
//	// Set view matrix
//	m_RenderTargetPtr->SetTransform(m_MatView.ToMatrix3x2F());
//	// Draw the hitregion
//	m_RenderTargetPtr->DrawGeometry(collisionMeshPtr->GetTransformedGeometry(), m_ColorBrushPtr);
//
//	//restore the world matrix
//	m_RenderTargetPtr->SetTransform((m_MatWorld * m_MatView).ToMatrix3x2F());
//	return true;
//}
//
//bool GameEngine::FillHitRegion(HitRegion* collisionMeshPtr)
//{
//	if (!CanIPaint()) return false;
//	// Set view matrix
//	m_RenderTargetPtr->SetTransform(m_MatView.ToMatrix3x2F());
//
//	// Draw the hitregion
//	m_RenderTargetPtr->FillGeometry(collisionMeshPtr->GetTransformedGeometry(), m_ColorBrushPtr);
//
//	//restore the world matrix
//	m_RenderTargetPtr->SetTransform((m_MatWorld * m_MatView).ToMatrix3x2F());
//	return true;
//}

//world matrix operations
void GameEngine::set_world_matrix(const MATRIX3X2& mat)
{
	_mat_world = mat;
	D2D1::Matrix3x2F matDirect2D = (_mat_world * _mat_view).ToMatrix3x2F();
	_d2d_rt->SetTransform(matDirect2D);
}

MATRIX3X2 GameEngine::get_world_matrix()
{
	return _mat_world;
}

//view matrix operations
void GameEngine::set_view_matrix(const MATRIX3X2& mat)
{
	_mat_view = mat;
	D2D1::Matrix3x2F matDirect2D = (_mat_world * _mat_view).ToMatrix3x2F();
	_d2d_rt->SetTransform(matDirect2D);
}

MATRIX3X2 GameEngine::get_view_matrix()
{
	return _mat_view;
}

void GameEngine::set_bitmap_interpolation_mode(bitmap_interpolation_mode mode)
{
	switch (mode)
	{
	case bitmap_interpolation_mode::linear:
		_hw_bitmap_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
		break;
	case bitmap_interpolation_mode::nearest_neighbor:
		_hw_bitmap_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
		break;
	default:
		assert("Case not supported");
		break;
	}

	_bitmap_interpolation_mode = mode;
}

void GameEngine::enable_aa(bool isEnabled)
{
	_aa_desc.Count = isEnabled ? 4 : 1;
	_aa_desc.Quality = isEnabled ? D3D11_CENTER_MULTISAMPLE_PATTERN : 0;

	//TODO: Request swapchain recreation

	_d2d_aa_mode = isEnabled ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	if(_d2d_rt)
	_d2d_rt->SetAntialiasMode(_d2d_aa_mode);
}

void GameEngine::enable_physics_debug_rendering(bool isEnabled)
{
	_debug_physics_rendering = isEnabled;
}

void GameEngine::set_font(Font* fontPtr)
{
	_user_font = fontPtr;
}

Font* GameEngine::get_font()
{
	return _user_font;
}

void GameEngine::set_default_font()
{
	_user_font = _default_font;
}

void GameEngine::register_gui(GUIBase *guiPtr)
{
	_gui_elements.push_back(guiPtr);
}

void GameEngine::unregister_gui(GUIBase *targetPtr)
{
	std::vector<GUIBase*>::iterator pos = find(_gui_elements.begin(), _gui_elements.end(), targetPtr); // find algorithm from STL

	if (pos == _gui_elements.end()) return;
	else
	{
		_gui_elements.erase(pos);
		return;
	}
}

HINSTANCE GameEngine::get_instance() const
{
	return _hinstance;
}

HWND GameEngine::get_window() const
{
	return _hwindow;
}

String GameEngine::get_title() const
{
	//return *m_Title; 
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

ImVec2 GameEngine::get_viewport_size(int id ) const
{
	assert(id == 0);
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

XMFLOAT2 GameEngine::get_mouse_pos_in_viewport()const
{
	return XMFLOAT2{ (float)_input_manager->GetMousePosition().x, (float)_input_manager->GetMousePosition().y};
}

AudioSystem * GameEngine::GetXAudio() const
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

void GameEngine::GUITick(double deltaTime)
{
	for (GUIBase* guiPtr : _gui_elements)
	{
		guiPtr->Tick(deltaTime);
	}
}

void GameEngine::GUIPaint()
{
	for (GUIBase* guiPtr : _gui_elements)
	{
		guiPtr->Paint();
	}
}

void GameEngine::GUIConsumeEvents()
{
	for (GUIBase* guiPtr : _gui_elements)
	{
		guiPtr->ConsumeEvent();
	}

}

void GameEngine::apply_settings(GameSettings &gameSettings)
{
	set_width(gameSettings.m_WindowWidth);
	set_height(gameSettings.m_WindowHeight);
	set_title(gameSettings.m_WindowTitle);
	enable_vsync(gameSettings.m_WindowFlags & GameSettings::WindowFlags::EnableVSync);
	enable_aa(gameSettings.m_WindowFlags & GameSettings::WindowFlags::EnableAA);

	if (gameSettings.m_WindowFlags & GameSettings::WindowFlags::EnableConsole)
	{
		console_create();
	}
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
		set_window(hWindow);
		return 0;
	case WM_SYSCOMMAND:	// trapping this message prevents a freeze after the ALT key is released
		if (wParam == SC_KEYMENU) return 0;			// see win32 API : WM_KEYDOWN
		else break;

	case WM_DESTROY:
		// End the game and exit the application
		GameEngine::instance()->_should_quit = true;
		PostQuitMessage(0);
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

	if (msg == WM_CHAR)
	{
		for (GUIBase* guiPtr : _gui_elements)
		{
			guiPtr->HandleKeyInput((TCHAR)wParam);
		}
	}


	return DefWindowProc(hWindow, msg, wParam, lParam);
}

// Create resources which are not bound
// to any device. Their lifetime effectively extends for the
// duration of the app. These resources include the Direct2D and
// DirectWrite factories,  and a DirectWrite Text Format object
// (used for identifying particular font characteristics).
void GameEngine::CreateDeviceIndependentResources()
{
	// Create Direct3D 11 factory
	{
		CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&_dxgi_factory);

		D3D_FEATURE_LEVEL levels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL featureLevel;
		SUCCEEDED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, &levels, 1, D3D11_SDK_VERSION, &_d3d_device, &featureLevel, &_d3d_device_ctx));
	}

	CreateD2DFactory();
	CreateWICFactory();
	CreateWriteFactory();

	_d3d_device_ctx->QueryInterface(IID_PPV_ARGS(&_d3d_user_defined_annotation));
}

void GameEngine::CreateD2DFactory()
{
	HRESULT hr;
	// Create a Direct2D factory.
	ID2D1Factory* localD2DFactoryPtr = nullptr;
	if (!_d2d_factory)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &localD2DFactoryPtr);
		if (FAILED(hr))
		{
			message_box(String("Create D2D Factory Failed"));
			exit(-1);
		}
		_d2d_factory = localD2DFactoryPtr;
	}
}

void GameEngine::CreateWICFactory()
{
	HRESULT hr;
	// Create a WIC factory if it does not exists
	IWICImagingFactory* localWICFactoryPtr = nullptr;
	if (!_wic_factory)
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&localWICFactoryPtr));
		if (FAILED(hr))
		{
			message_box(String("Create WIC Factory Failed"));
			exit(-1);
		}
		_wic_factory = localWICFactoryPtr;
	}
}

void GameEngine::CreateWriteFactory()
{
	HRESULT hr;
	// Create a DirectWrite factory.
	IDWriteFactory* localDWriteFactoryPtr = nullptr;
	if (!_dwrite_factory)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(localDWriteFactoryPtr), reinterpret_cast<IUnknown **>(&localDWriteFactoryPtr));
		if (FAILED(hr))
		{
			message_box(String("Create WRITE Factory Failed"));
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
void GameEngine::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!_dxgi_swapchain)
	{
		DXGI_SWAP_CHAIN_DESC desc{};
		desc.BufferDesc.Width = get_width();
		desc.BufferDesc.Height = get_height();
		desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		desc.OutputWindow = get_window();
		if (_game_settings.m_FullscreenMode == GameSettings::FullScreenMode::Windowed || _game_settings.m_FullscreenMode == GameSettings::FullScreenMode::BorderlessWindowed)
		{
			desc.Windowed = TRUE;
		}
		else
		{
			desc.Windowed = false;
		}
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SUCCEEDED(_dxgi_factory->CreateSwapChain(_d3d_device, &desc, &_dxgi_swapchain));

		set_debug_name(_dxgi_swapchain, "DXGISwapchain");
		set_debug_name(_dxgi_factory, "DXGIFactory");

		ID3D11Texture2D* backBuffer;
		_dxgi_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		set_debug_name(backBuffer, "DXGIBackBuffer");
		assert(backBuffer);
		SUCCEEDED(_d3d_device->CreateRenderTargetView(backBuffer, NULL, &_d3d_backbuffer_view));
		SUCCEEDED(_d3d_device->CreateShaderResourceView(backBuffer, NULL, &_d3d_backbuffer_srv));
		backBuffer->Release();

		UINT dpi = GetDpiForWindow(get_window());
		D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);

		IDXGISurface* surface;
		backBuffer->QueryInterface(&surface);

		hr = _d2d_factory->CreateDxgiSurfaceRenderTarget(surface, rtp, &_d2d_rt);
		surface->Release();

		if (FAILED(hr))
		{
			message_box(String("Create CreateDeviceResources Failed"));
			exit(-1);
		}

		//set alias mode
		_d2d_rt->SetAntialiasMode(_d2d_aa_mode);

		// Create a brush.
		_d2d_rt->CreateSolidColorBrush((D2D1::ColorF) D2D1::ColorF::Black, &_color_brush);

		//Create a Font
		_default_font = new Font(String("Consolas"), 12);
		_user_font = _default_font;
		_initialized = true;
	}
}

//
//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
//
void GameEngine::DiscardDeviceResources()
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

	_dxgi_swapchain->Release();
	_dxgi_swapchain = nullptr;
}

void GameEngine::d2d_begin_paint()
{
	if (_d2d_rt)
	{
		_d2d_rt->BeginDraw();
		_d2d_rt->SetTransform(D2D1::Matrix3x2F::Identity());

		// set black as initial brush color 
		_color_brush->SetColor(D2D1::ColorF((FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(1.0)));
	}
}

bool GameEngine::d2d_end_paint()
{
	HRESULT hr = S_OK;
	hr = _d2d_rt->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET)
	{
		message_box(String(" Direct2D error: RenderTarget lost.\nThe GameEngine terminates the game.\n"));
		return false; //app should close or re-initialize
	}
	return true;
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

void GameEngine::PreSolve(b2Contact* contactPtr, const b2Manifold* oldManifoldPtr)
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
		ImGuiIO& io = ImGui::GetIO();
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
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

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


