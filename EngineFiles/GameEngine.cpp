#include "stdafx.h"
#include "../stdafx.h"

#include "GameEngine.h"
#include "ContactListener.h"
#include "AbstractGame.h"

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "DebugOverlays/MetricsOverlay.h"
#include "DebugOverlays/RTTIDebugOverlay.h"
#include "DebugOverlays/ImGuiOverlays.h"

#include "Core/ResourceLoader.h"
#include "Core/logging.h"

enki::TaskScheduler game_engine::s_TaskScheduler;
std::thread::id game_engine::s_MainThread;

//-----------------------------------------------------------------
// Windows Functions
//-----------------------------------------------------------------
LRESULT CALLBACK game_engine::WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route all Windows messages to the game engine
	return game_engine::instance()->handle_event(hWindow, msg, wParam, lParam);
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
game_engine::game_engine() : 
	m_hInstance(0),
	m_hWindow(NULL),
	m_wIcon(0),
	m_wSmallIcon(0), //changed in june 2014, reset to false in dec 2014
	m_iWidth(0), m_iHeight(0),
	m_bSleep(true), m_dKeybThreadID(0), m_GamePtr(nullptr),
	m_ConsoleHandle(NULL),
	m_bPaintingAllowed(false),
	m_bVSync(true),
	m_bInitialized(false),
	m_DXGIFactoryPtr(nullptr),
	m_D3DDevicePtr(nullptr),
	m_D3DDeviceContextPtr(nullptr),
	m_DXGISwapchainPtr(nullptr),
	m_D2DFactoryPtr(nullptr),
	m_WICFactoryPtr(nullptr),
	m_RenderTargetPtr(nullptr),
	m_DWriteFactoryPtr(nullptr),
	m_GameTickTimerPtr(nullptr),
	m_ColorBrushPtr(nullptr),
	m_AADesc({ 1,0 }),
	m_hw_bitmap_interpolation_mode(D2D1_BITMAP_INTERPOLATION_MODE_LINEAR),
	m_DefaultFontPtr(nullptr),
	m_UserFontPtr(nullptr),
	m_InputPtr(nullptr),
	m_XaudioPtr(nullptr),
	m_GameSettings(),
	m_PhysicsStepEnabled(true),
	m_bQuit(false),
	m_ViewportFocused(false),
	_recreate_swapchain(false),
	_recreate_game_texture(false)
{
	m_Gravity = DOUBLE2(0, 9.81);

	// Seed the random number generator
	srand(GetTickCount());

	// Initialize Direct2D system
	CoInitialize(0);
	CreateDeviceIndependentResources();

	m_OverlayManager = std::make_shared<OverlayManager>();
	m_MetricsOverlay = new MetricsOverlay();

	m_OverlayManager->register_overlay(m_MetricsOverlay);
	m_OverlayManager->register_overlay(new RTTIDebugOverlay());
	m_OverlayManager->register_overlay(new ImGuiDemoOverlay());
	m_OverlayManager->register_overlay(new ImGuiAboutOverlay());

	// Start up the keyboard thread
	//m_hKeybThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) ::KeybThreadProc, this, NULL, &m_dKeybThreadID);
}

game_engine::~game_engine()
{

	//Free the console
	if (m_ConsoleHandle)
	{
		m_ConsoleHandle = NULL;
	}

	delete m_InputPtr;
	delete m_GameTickTimerPtr;
	delete m_GamePtr;
	delete m_DefaultFontPtr;
    delete m_Box2DContactFilter;

	//Direct2D Device dependent related stuff
	DiscardDeviceResources();

	//Direct2D Device independent related stuff
	m_D3DDeviceContextPtr->Release();
	m_D3DDevicePtr->Release();
	m_DXGIFactoryPtr->Release();

	m_DWriteFactoryPtr->Release();
	m_WICFactoryPtr->Release();
	m_D2DFactoryPtr->Release();

#ifndef WINDOWS7
	delete m_XaudioPtr;
#endif

	CoUninitialize();
}

void game_engine::set_game(AbstractGame* gamePtr)
{
	m_GamePtr = gamePtr;
}

//-----------------------------------------------------------------
// Game Engine General Methods
//-----------------------------------------------------------------
void game_engine::set_title(const String& titleRef)
{
	m_Title = titleRef;
}


int game_engine::run(HINSTANCE hInstance, int iCmdShow)
{
	s_MainThread = std::this_thread::get_id();

	// Initialize some windows stuff
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))

	// create the game engine object, exit if failure
	assert(game_engine::instance());

	// set the instance member variable of the game engine
	this->set_instance(hInstance);

	// Initialize enkiTS
	// TODO: Hookup profiler callbacks
	s_TaskScheduler.Initialize();

	//Initialize the high precision timers
	m_GameTickTimerPtr = new PrecisionTimer();
	m_GameTickTimerPtr->Reset();

	// Inputmanager
	m_InputPtr = new InputManager();
	m_InputPtr->Initialize();

	// Sound system
#ifndef WINDOWS7
	m_XaudioPtr = new AudioSystem();
#endif

	// Game Initialization
	m_GamePtr->GameInitialize(m_GameSettings);
	apply_settings(m_GameSettings);

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
	ImGui_ImplDX11_Init(m_D3DDevicePtr, m_D3DDeviceContextPtr);
#pragma region Box2D
	// Initialize Box2D
	// Define the gravity vector.
	b2Vec2 gravity((float)m_Gravity.x, (float)m_Gravity.y);

	// Construct a world object, which will hold and simulate the rigid bodies.
	m_Box2DWorldPtr = new b2World(gravity);
    m_Box2DContactFilter = new b2ContactFilter();
    
    m_Box2DWorldPtr->SetContactFilter(m_Box2DContactFilter);
	//m_Box2DWorldPtr->SetContactListener(m_GamePtr);
	m_Box2DWorldPtr->SetContactListener(this);

	m_Box2DDebugRenderer.SetFlags(b2Draw::e_shapeBit);
	m_Box2DDebugRenderer.AppendFlags(b2Draw::e_centerOfMassBit);
	m_Box2DDebugRenderer.AppendFlags(b2Draw::e_jointBit);
	m_Box2DDebugRenderer.AppendFlags(b2Draw::e_pairBit);
	m_Box2DWorldPtr->SetDebugDraw(&m_Box2DDebugRenderer);
#pragma endregion


	// User defined functions for start of the game
	m_GamePtr->GameStart();

	//// Sleep is evil: default Sleep(1) actually sleeps for 14 msecs !!!
	//// http://msdn.microsoft.com/en-us/library/windows/desktop/ms686307(v=vs.85).aspx
	#include <Mmsystem.h>
	#pragma comment (lib, "Winmm.lib")
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
	double previous = m_GameTickTimerPtr->GetGameTime() - m_PhysicsTimeStep;
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
		if (m_bQuit)
			break;

		{
			++m_FrameCounter;

			double current = m_GameTickTimerPtr->GetGameTime();
			double elapsed = current - previous; // calc timedifference
			m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::FrameTime, (float)(elapsed * 1000.0f));
			if (elapsed > 0.25) elapsed = 0.25; //prevent jumps in time when break point or sleeping
			previous = current;  // reset
			lag += elapsed;

			Timer t{};
			t.Start();
			while (lag >= m_PhysicsTimeStep)
			{
				// Check the state of keyboard and mouse
				m_InputPtr->Update();

				//tick GUI -> for blinking caret
				GUITick(m_PhysicsTimeStep);

				// Call the Game Tick method
				m_GamePtr->GameTick(m_PhysicsTimeStep);

				int32 velocityIterations = 6;
				int32 positionIterations = 2;
				if (m_PhysicsStepEnabled)
				{
					m_Box2DWorldPtr->Step((float)m_PhysicsTimeStep, velocityIterations, positionIterations);
				}

				// Step generates contact lists, pass to Listeners and clear the vector
				CallListeners();
				lag -= m_PhysicsTimeStep;
			}
			t.Stop();
			m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::GameUpdateCPU, (float)t.GetTimeInMS());

			if (_recreate_swapchain)
			{
				logging::logf("Recreating swapchain. New size: %dx%d\n", (uint32_t)m_iWidth, (uint32_t)m_iHeight);

				this->resize_swapchain(m_iWidth, m_iHeight);
				_recreate_swapchain = false;
			}

			// Recreating the game viewport texture needs to happen before running IMGUI and the actual rendering
			if (_recreate_game_texture)
			{
				logging::logf("Recreating game texture. New size: %dx%d\n", (uint32_t)_game_viewport_size.x, (uint32_t)_game_viewport_size.y);

				this->resize_game_view(_game_viewport_size.x, _game_viewport_size.y);
				_recreate_game_texture = false;
			}

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			build_ui();
			{
				m_GamePtr->DebugUI();
				m_OverlayManager->render_overlay();

				ImVec2 game_width = { get_width() / 2.0f, get_height() / 2.0f };
				ImGui::SetNextWindowSize(game_width, ImGuiCond_FirstUseEver);
				ImGui::Begin("Viewport");
				{
					m_ViewportFocused = ImGui::IsWindowFocused();
					ImVec2 size = ImGui::GetContentRegionAvail();
					if (_game_viewport_size.x != size.x || _game_viewport_size.y != size.y)
					{
						_recreate_game_texture = true;
						_game_viewport_size.x = std::max(size.x, 4.f);
						_game_viewport_size.y = std::max(size.y, 4.f);
					}

					ImGui::GetWindowDrawList()->AddImage(_game_output_srv, ImVec2(0.0, 0.0), ImVec2(1.0, 1.0));
					ImVec2 actual_size{ ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y };
					ImGui::Image(_game_output_srv, actual_size);
				}
				ImGui::End();
			}
			ImGui::EndFrame();
			ImGui::UpdatePlatformWindows();
			ImGui::Render();

			// Get gpu data 
			size_t idx = m_FrameCounter % 2;
			if (m_FrameCounter > 2)
			{
				size_t prev_idx = (m_FrameCounter - 1) % 2;
				D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestampDisjoint;
				UINT64 start;
				UINT64 end;
				while (S_OK != m_D3DDeviceContextPtr->GetData(gpuTimings[idx][0], &timestampDisjoint, sizeof(timestampDisjoint), 0)) {}
				while (S_OK != m_D3DDeviceContextPtr->GetData(gpuTimings[idx][1], &start, sizeof(UINT64), 0)) {}
				while (S_OK != m_D3DDeviceContextPtr->GetData(gpuTimings[idx][2], &end, sizeof(UINT64), 0)) {}

				double diff = (double)(end - start) / (double)timestampDisjoint.Frequency;
				m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::RenderGPU, (float)(diff * 1000.0));
			}
		}

		ResourceLoader::instance()->update();

		GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"Frame");



		size_t idx = m_FrameCounter % 2;

		m_D3DDeviceContextPtr->Begin(gpuTimings[idx][0]);
		m_D3DDeviceContextPtr->End(gpuTimings[idx][1]);

		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<float>(this->get_viewport_size().x);
		vp.Height = static_cast<float>(this->get_viewport_size().y);
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		m_D3DDeviceContextPtr->RSSetViewports(1, &vp);

		FLOAT color[4] = { 0.25f,0.25,0.25f,1.0f };
		m_D3DDeviceContextPtr->ClearRenderTargetView(_game_output_rtv, color);
		m_D3DDeviceContextPtr->ClearDepthStencilView(_game_output_dsv, D3D11_CLEAR_DEPTH, 0.0f, 0);
		m_D3DDeviceContextPtr->OMSetRenderTargets(1, &_game_output_rtv, _game_output_dsv);

		// Render 3D before 2D
		{
			GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"Render3D");
			m_GamePtr->Render3D();
		}

		// Render Direct2D to the swapchain
		ExecuteDirect2DPaint();

		// Render main viewport ImGui

		m_D3DDeviceContextPtr->ClearRenderTargetView(m_D3DBackBufferView, color);
		m_D3DDeviceContextPtr->OMSetRenderTargets(1, &m_D3DBackBufferView, nullptr);

		{
			GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"ImGui");
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		// Present
		GPU_MARKER(m_D3DUserDefinedAnnotation, L"DrawEnd");
		m_DXGISwapchainPtr->Present(m_bVSync ? 1 : 0, 0);



		m_D3DDeviceContextPtr->End(gpuTimings[idx][2]);
		m_D3DDeviceContextPtr->End(gpuTimings[idx][0]);

		// Render all other imgui windows  
		ImGui::RenderPlatformWindowsDefault();
	}
	// undo the timer setting
	timeEndPeriod(tc.wPeriodMin);

	// Make sure all tasks have finished before shutting down
	s_TaskScheduler.WaitforAllAndShutdown();

	// User defined code for exiting the game
	m_GamePtr->GameEnd();

	ResourceLoader::instance()->unload_all();
	ResourceLoader::Shutdown();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	// Box2D
	delete m_Box2DWorldPtr;

	return 0;
}

void game_engine::ExecuteDirect2DPaint()
{
	GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"Game2D");

	D2DBeginPaint();
	RECT usedClientRect = { 0, 0, _game_viewport_size.x, _game_viewport_size.y };

	m_bPaintingAllowed = true;
	// make sure the view matrix is taken in account
	SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
	m_GamePtr->GamePaint(usedClientRect);

	//Paint the buttons and textboxes
	GUIPaint();

	// draw Box2D debug rendering
	// http://www.iforce2d.net/b2dtut/debug-draw
	if (m_DebugRendering)
	{
		// dimming rect in screenspace
		SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
		MATRIX3X2 matView = GetViewMatrix();
		SetViewMatrix(MATRIX3X2::CreateIdentityMatrix());
		set_color(COLOR(0, 0, 0, 127));
		FillRect(0, 0, get_width(), get_height());
		SetViewMatrix(matView);
		m_Box2DWorldPtr->DrawDebugData();
	}

	// deactivate all gui objects
	GUIConsumeEvents();

	m_bPaintingAllowed = false;
	bool result = D2DEndPaint();

	// if drawing failed, terminate the game
	if (!result) PostMessage(game_engine::get_window(), WM_DESTROY, 0, 0);
}

bool game_engine::register_wnd_class()
{
	WNDCLASSEX wndclass;

	// Create the window class for the main window
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = m_hInstance;
	wndclass.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(get_icon()));
	wndclass.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(get_small_icon()));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = m_Title.C_str();

	// Register the window class
	if (!RegisterClassEx(&wndclass)) return false;
	return true;
}

bool game_engine::open_window(int iCmdShow)
{
	// Calculate the window size and position based upon the game size
	DWORD windowStyle = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_OVERLAPPEDWINDOW;
	RECT R = { 0, 0, m_iWidth, m_iHeight };
	AdjustWindowRect(&R, windowStyle, false);
	int iWindowWidth = R.right - R.left;
	int iWindowHeight = R.bottom - R.top;
	int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2;
	int iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;

	m_hWindow = CreateWindow(m_Title.C_str(), m_Title.C_str(),
		windowStyle,
		iXWindowPos, iYWindowPos, iWindowWidth,
		iWindowHeight, NULL, NULL, m_hInstance, NULL);

	if (!m_hWindow) return false;

	// Show and update the window
	if (m_GameSettings.m_WindowFlags & GameSettings::WindowFlags::StartMaximized)
		iCmdShow = SW_SHOWMAXIMIZED;

	ShowWindow(m_hWindow, iCmdShow);
	UpdateWindow(m_hWindow);

	// Update size

	RECT r;
	::GetClientRect(m_hWindow, &r);
	m_iWidth = r.right - r.left;
	m_iHeight = r.bottom - r.top;

	return true;
}

void game_engine::resize_swapchain(uint32_t width, uint32_t height)
{

	// Resize the swapchain
	if (m_D3DBackBufferView) m_D3DBackBufferView->Release();
	if (m_D3DBackBufferSRV)  m_D3DBackBufferSRV->Release();

	DXGI_SWAP_CHAIN_DESC desc;
	m_DXGISwapchainPtr->GetDesc(&desc);
	m_DXGISwapchainPtr->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, desc.Flags);

	// Recreate the views
	ID3D11Texture2D* backBuffer;
	m_DXGISwapchainPtr->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	assert(backBuffer);
	SUCCEEDED(m_D3DDevicePtr->CreateRenderTargetView(backBuffer, NULL, &m_D3DBackBufferView));
	SUCCEEDED(m_D3DDevicePtr->CreateShaderResourceView(backBuffer, NULL, &m_D3DBackBufferSRV));
	backBuffer->Release();
}

void game_engine::resize_game_view(uint32_t width, uint32_t height)
{
	m_D3DDeviceContextPtr->Flush();

	if (_game_output_tex)
		_game_output_tex->Release();
	if (_game_output_srv)
		_game_output_rtv->Release();
	if (_game_output_srv)
		_game_output_srv->Release();
	if (_game_output_dsv)
		_game_output_dsv->Release();
	if (_game_output_depth)
		_game_output_depth->Release();

	if (m_RenderTargetPtr)
		m_RenderTargetPtr->Release();


	{
		CD3D11_TEXTURE2D_DESC texture_desc{ DXGI_FORMAT_R8G8B8A8_UNORM, (UINT)_game_viewport_size.x, (UINT)_game_viewport_size.y };
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;

		SUCCEEDED(m_D3DDevicePtr->CreateTexture2D(&texture_desc, NULL, &_game_output_tex));

		texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		SUCCEEDED(m_D3DDevicePtr->CreateTexture2D(&texture_desc, NULL, &_game_output_depth));

		// Create the views
		SUCCEEDED(m_D3DDevicePtr->CreateDepthStencilView(_game_output_depth, NULL, &_game_output_dsv));
		SUCCEEDED(m_D3DDevicePtr->CreateRenderTargetView(_game_output_tex, NULL, &_game_output_rtv));
		SUCCEEDED(m_D3DDevicePtr->CreateShaderResourceView(_game_output_tex, NULL, &_game_output_srv));


		UINT dpi = GetDpiForWindow(get_window());
		D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);
		IDXGISurface* surface;
		_game_output_tex->QueryInterface(&surface);
		SUCCEEDED(m_D2DFactoryPtr->CreateDxgiSurfaceRenderTarget(surface, rtp, &m_RenderTargetPtr));
		surface->Release();

	}
}

void game_engine::quit_game()
{
	PostMessage(game_engine::get_window(), WM_DESTROY, 0, 0);
}

void game_engine::message_box(const String &text) const
{
	if (sizeof(TCHAR) == 2)	::MessageBoxW(get_window(),(wchar_t*)text.C_str(), (wchar_t*)m_Title.C_str(), MB_ICONEXCLAMATION | MB_OK);
	else MessageBoxA(get_window(), (char*)text.C_str(), (char*)m_Title.C_str(), MB_ICONEXCLAMATION | MB_OK);
}

void game_engine::console_create()
{
	if (m_ConsoleHandle == NULL && AllocConsole())
	{
		//get the console handle
		m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		//set new console title
		SetConsoleTitle((String("Console ") + m_Title).C_str());
		// STDOUT redirection
		int  conHandle = _open_osfhandle(PtrToLong(m_ConsoleHandle), 0x4000);
		FILE* fp = _fdopen(conHandle, "w");
		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);
	}
}

void game_engine::ConsoleSetForeColor(bool red, bool green, bool blue, bool intensity)
{
	//retrieve current color settings
	CONSOLE_SCREEN_BUFFER_INFO buffer = {};
	GetConsoleScreenBufferInfo(m_ConsoleHandle, &buffer);

	//copy the background color attributes
	WORD wAttributes = buffer.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	//set the fore color attributes
	if (red) wAttributes |= FOREGROUND_RED;
	if (green) wAttributes |= FOREGROUND_GREEN;
	if (blue) wAttributes |= FOREGROUND_BLUE;
	if (intensity) wAttributes |= FOREGROUND_INTENSITY;
	//set the new color attributes to the console
	SetConsoleTextAttribute(m_ConsoleHandle, wAttributes);
}

void game_engine::ConsoleSetBackColor(bool red, bool green, bool blue, bool intensity)
{
	//retrieve current color settings
	CONSOLE_SCREEN_BUFFER_INFO buffer = {};
	GetConsoleScreenBufferInfo(m_ConsoleHandle, &buffer);

	//copy the fore color attributes
	WORD wAttributes = buffer.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	//set the back color attributes
	if (red) wAttributes |= BACKGROUND_RED;
	if (green) wAttributes |= BACKGROUND_GREEN;
	if (blue) wAttributes |= BACKGROUND_BLUE;
	if (intensity) wAttributes |= BACKGROUND_INTENSITY;
	//set the new color attributes to the console
	SetConsoleTextAttribute(m_ConsoleHandle, wAttributes);
}

void game_engine::ConsoleSetCursorPosition(int column, int row)
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
	SetConsoleCursorPosition(m_ConsoleHandle, myUnion.COORD);
}

void game_engine::ConsolePrintString(const String& textRef)
{
#ifdef _UNICODE
	std::wcout << textRef.C_str() << "\n";
#else
	std::cout << textRef.C_str() << "\n";
#endif
	String copy = textRef + String("\n");
	::OutputDebugString(copy);
}

void game_engine::ConsolePrintString(std::string const& msg)
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

void game_engine::ConsolePrintString(const String& textRef, int column, int row)
{
	ConsoleSetCursorPosition(column, row);
	ConsolePrintString(textRef);
}

//void GameEngine::ConsolePrintString(string text, int column, int row) 
//{
//	ConsoleSetCursorPosition(column,  row);
//	ConsolePrintString(text);
//}

void game_engine::console_clear() const
{
	system("cls");
}

void game_engine::set_instance(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}

void game_engine::set_window(HWND hWindow)
{
	m_hWindow = hWindow;
}

bool game_engine::is_paint_allowed() const
{
	if (m_bPaintingAllowed) return true;
	else
	{
#ifdef _DEBUG
		MessageBoxA(NULL, "Painting from outside the GamePaint()...\n\nYOU SHALL NOT PASS!!!", "GameEngine says NO", MB_OK);
#endif
		return false;
	}
}

void game_engine::set_color(COLOR color)
{
	m_ColorBrushPtr->SetColor(D2D1::ColorF((FLOAT)(color.red / 255.0), (FLOAT)(color.green / 255.0), (FLOAT)(color.blue / 255.0), (FLOAT)(color.alpha / 255.0)));
}

COLOR game_engine::get_color()
{
	D2D1_COLOR_F dColor = m_ColorBrushPtr->GetColor();
	return COLOR((unsigned char)(dColor.r * 255), (unsigned char)(dColor.g * 255), (unsigned char)(dColor.b * 255), (unsigned char)(dColor.a * 255));
}

bool game_engine::DrawSolidBackground(COLOR backgroundColor)
{
	if (!is_paint_allowed()) return false;

	m_RenderTargetPtr->Clear(D2D1::ColorF((FLOAT)(backgroundColor.red / 255.0), (FLOAT)(backgroundColor.green / 255.0), (FLOAT)(backgroundColor.blue / 255.0), (FLOAT)(backgroundColor.alpha)));

	return true;
}

bool game_engine::DrawLine(int x1, int y1, int x2, int y2)
{
	return DrawLine(DOUBLE2(x1, y1), DOUBLE2(x2, y2), 1.0);
}

bool game_engine::DrawLine(DOUBLE2 p1, DOUBLE2 p2, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	m_RenderTargetPtr->DrawLine(Point2F((FLOAT)p1.x, (FLOAT)p1.y), Point2F((FLOAT)p2.x, (FLOAT)p2.y), m_ColorBrushPtr, (FLOAT)strokeWidth);

	return true;
}

bool game_engine::DrawPolygon(const std::vector<POINT>& ptsArr, unsigned int count, bool close)
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

bool game_engine::DrawPolygon(const std::vector<DOUBLE2>& ptsArr, unsigned int count, bool close, double strokeWidth)
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

bool game_engine::FillPolygon(const std::vector<POINT>& ptsArr, unsigned int count)
{
	if (!is_paint_allowed()) return false;
	//unsigned int count = ptsArr.size();

	//do not fill an empty polygon
	if (count<2)return false;

	HRESULT hr;

	// Create path geometry
	ID2D1PathGeometry *geometryPtr;
	hr = m_D2DFactoryPtr->CreatePathGeometry(&geometryPtr);
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
		m_RenderTargetPtr->FillGeometry(geometryPtr, m_ColorBrushPtr);
		geometryPtr->Release();
		return true;
	}

	geometryPtr->Release();
	return false;
}

bool game_engine::FillPolygon(const std::vector<DOUBLE2>& ptsArr, unsigned int count)
{
	if (!is_paint_allowed())return false;
	//unsigned int count = ptsArr.size();

	//do not fill an empty polygon
	if (count<2)return false;

	HRESULT hr;

	// Create path geometry
	ID2D1PathGeometry *geometryPtr;
	hr = m_D2DFactoryPtr->CreatePathGeometry(&(geometryPtr));
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
		m_RenderTargetPtr->FillGeometry(geometryPtr, m_ColorBrushPtr);
		geometryPtr->Release();
		return true;
	}

	geometryPtr->Release();
	return false;
}

bool game_engine::DrawRect(int left, int top, int right, int bottom)
{
	RECT2 rect2(left, top, right, bottom);
	return DrawRect(rect2, 1.0);
}

bool game_engine::DrawRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, double strokeWidth)
{
	RECT2 rect2(topLeft.x, topLeft.y, rightbottom.x, rightbottom.y);
	return DrawRect(rect2, strokeWidth);
}

bool game_engine::DrawRect(RECT rect)
{
	RECT2 rect2(rect.left, rect.top, rect.right, rect.bottom);
	return DrawRect(rect2, 1.0);
}

bool game_engine::DrawRect(RECT2 rect, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) MessageBoxA(NULL, "GameEngine::DrawRect error: invalid dimensions!", "GameEngine says NO", MB_OK);
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	m_RenderTargetPtr->DrawRectangle(d2dRect, m_ColorBrushPtr, (FLOAT)strokeWidth);

	return true;
}

bool game_engine::FillRect(int left, int top, int right, int bottom)
{
	RECT2 rect2(left, top, right, bottom);
	return FillRect(rect2);
}

bool game_engine::FillRect(DOUBLE2 topLeft, DOUBLE2 rightbottom)
{
	RECT2 rect2(topLeft.x, topLeft.y, rightbottom.x, rightbottom.y);
	return FillRect(rect2);
}

bool game_engine::FillRect(RECT rect)
{
	RECT2 rect2(rect.left, rect.top, rect.right, rect.bottom);
	return FillRect(rect2);
}

bool game_engine::FillRect(RECT2 rect)
{
	if (!is_paint_allowed()) return false;
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) MessageBoxA(NULL, "GameEngine::DrawRect error: invalid dimensions!", "GameEngine says NO", MB_OK);

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	m_RenderTargetPtr->FillRectangle(d2dRect, m_ColorBrushPtr);

	return true;
}

bool game_engine::DrawRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, 1.0);
	return true;
}

bool game_engine::DrawRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, (FLOAT)strokeWidth);
	return true;
}

bool game_engine::DrawRoundedRect(RECT rect, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, 1.0);
	return true;
}

bool game_engine::DrawRoundedRect(RECT2 rect, int radiusX, int radiusY, double strokeWidth)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, (FLOAT)strokeWidth);
	return true;
}

bool game_engine::FillRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool game_engine::FillRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool game_engine::FillRoundedRect(RECT rect, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool game_engine::FillRoundedRect(RECT2 rect, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool game_engine::DrawEllipse(int centerX, int centerY, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawEllipse(ellipse, m_ColorBrushPtr, 1.0);

	return true;
}

bool game_engine::DrawEllipse(DOUBLE2 centerPt, double radiusX, double radiusY, double strokeWidth)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawEllipse(ellipse, m_ColorBrushPtr, (FLOAT)strokeWidth);

	return true;
}

bool game_engine::FillEllipse(int centerX, int centerY, int radiusX, int radiusY)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillEllipse(ellipse, m_ColorBrushPtr);

	return true;
}

bool game_engine::FillEllipse(DOUBLE2 centerPt, double radiusX, double radiusY)
{
	if (!is_paint_allowed()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillEllipse(ellipse, m_ColorBrushPtr);

	return true;
}
bool game_engine::DrawString(const String& text, RECT boundingRect)
{
	return DrawString(text, boundingRect.left, boundingRect.top, boundingRect.right, boundingRect.bottom);
}

bool game_engine::DrawString(const String& text, RECT2 boundingRect)
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

bool game_engine::DrawString(const String& text, int left, int top, int right, int bottom)
{
	return DrawString(text, DOUBLE2(left, top), right, bottom);
}

bool game_engine::DrawString(const String& text, DOUBLE2 topLeft, double right, double bottom)
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

	m_RenderTargetPtr->DrawText(stext.c_str(), UINT32(stext.length()), m_UserFontPtr->GetTextFormat(), layoutRect, m_ColorBrushPtr, options);

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

bool game_engine::DrawBitmap(Bitmap* imagePtr)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return DrawBitmap(imagePtr, DOUBLE2(0, 0), srcRect2);
}

bool game_engine::DrawBitmap(Bitmap* imagePtr, RECT srcRect)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	return DrawBitmap(imagePtr, DOUBLE2(0, 0), srcRect2);
}

bool game_engine::DrawBitmap(Bitmap* imagePtr, int x, int y)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return DrawBitmap(imagePtr, DOUBLE2(x, y), srcRect2);
}

bool game_engine::DrawBitmap(Bitmap* imagePtr, int x, int y, RECT srcRect)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	return DrawBitmap(imagePtr, DOUBLE2(x, y), srcRect2);
}

bool game_engine::DrawBitmap(Bitmap* imagePtr, DOUBLE2 position)
{
	if (imagePtr == nullptr) MessageBoxA(NULL, "DrawBitmap called using a bitmap pointer that is a nullptr\nThe MessageBox that will appear after you close this MessageBox is the default error message from visual studio.", "GameEngine says NO", MB_OK);
	RECT2 srcRect2(0, 0, imagePtr->GetWidth(), imagePtr->GetHeight());
	return DrawBitmap(imagePtr, position, srcRect2);
}

bool game_engine::DrawBitmap(Bitmap* imagePtr, DOUBLE2 position, RECT2 srcRect)
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

	m_RenderTargetPtr->DrawBitmap(imagePtr->GetBitmapPtr(), dstRect_f, (FLOAT)imagePtr->GetOpacity(), m_hw_bitmap_interpolation_mode, srcRect_f);

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
void game_engine::SetWorldMatrix(const MATRIX3X2& mat)
{
	m_MatWorld = mat;
	D2D1::Matrix3x2F matDirect2D = (m_MatWorld * m_MatView).ToMatrix3x2F();
	m_RenderTargetPtr->SetTransform(matDirect2D);
}

MATRIX3X2 game_engine::GetWorldMatrix()
{
	return m_MatWorld;
}

//view matrix operations
void game_engine::SetViewMatrix(const MATRIX3X2& mat)
{
	m_MatView = mat;
	D2D1::Matrix3x2F matDirect2D = (m_MatWorld * m_MatView).ToMatrix3x2F();
	m_RenderTargetPtr->SetTransform(matDirect2D);
}

MATRIX3X2 game_engine::GetViewMatrix()
{
	return m_MatView;
}

void game_engine::set_bitmap_interpolation_mode(bitmap_interpolation_mode mode)
{
	switch (mode)
	{
	case bitmap_interpolation_mode::linear:
		m_hw_bitmap_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
		break;
	case bitmap_interpolation_mode::nearest_neighbor:
		m_hw_bitmap_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
		break;
	default:
		assert("Case not supported");
		break;
	}

	m_bitmap_interpolation_mode = mode;
}

void game_engine::enable_aa(bool isEnabled)
{
	m_AADesc.Count = isEnabled ? 4 : 1;
	m_AADesc.Quality = isEnabled ? D3D11_CENTER_MULTISAMPLE_PATTERN : 0;

	//TODO: Request swapchain recreation

	m_D2DAAMode = isEnabled ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	if(m_RenderTargetPtr)
	m_RenderTargetPtr->SetAntialiasMode(m_D2DAAMode);
}

void game_engine::EnablePhysicsDebugRendering(bool isEnabled)
{
	m_DebugRendering = isEnabled;
}

void game_engine::SetBitmapInterpolationModeLinear()
{
	m_hw_bitmap_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
}

void game_engine::SetBitmapInterpolationModeNearestNeighbor()
{
	m_hw_bitmap_interpolation_mode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
}

void game_engine::set_font(Font* fontPtr)
{
	m_UserFontPtr = fontPtr;
}

Font* game_engine::get_font()
{
	return m_UserFontPtr;
}

void game_engine::set_default_font()
{
	m_UserFontPtr = m_DefaultFontPtr;
}

void game_engine::register_gui(GUIBase *guiPtr)
{
	m_GUIPtrArr.push_back(guiPtr);
}

void game_engine::unregister_gui(GUIBase *targetPtr)
{
	std::vector<GUIBase*>::iterator pos = find(m_GUIPtrArr.begin(), m_GUIPtrArr.end(), targetPtr); // find algorithm from STL

	if (pos == m_GUIPtrArr.end()) return;
	else
	{
		m_GUIPtrArr.erase(pos);
		return;
	}
}

HINSTANCE game_engine::get_instance() const
{
	return m_hInstance;
}

HWND game_engine::get_window() const
{
	return m_hWindow;
}

String game_engine::get_title() const
{
	//return *m_Title; 
	return m_Title;
}

WORD game_engine::get_icon() const
{
	return m_wIcon;
}

WORD game_engine::get_small_icon() const
{
	return m_wSmallIcon;
}

ImVec2 game_engine::get_window_size() const
{
	return { (float)m_iWidth, (float)m_iHeight };
}

ImVec2 game_engine::get_viewport_size(int id ) const
{
	assert(id == 0);
	return _game_viewport_size;
}

int game_engine::get_width() const
{
	return m_iWidth;
}

int game_engine::get_height() const
{
	return m_iHeight;
}

bool game_engine::get_sleep() const
{
	return m_bSleep ? true : false;
}

ID3D11Device* game_engine::GetD3DDevice() const
{
	return m_D3DDevicePtr;
}

ID3D11DeviceContext* game_engine::GetD3DDeviceContext() const
{
	return m_D3DDeviceContextPtr;
}

ID3D11RenderTargetView* game_engine::GetD3DBackBufferView() const
{
	return m_D3DBackBufferView;
}

ID2D1Factory* game_engine::GetD2DFactory() const
{
	return m_D2DFactoryPtr;
}

IWICImagingFactory* game_engine::GetWICImagingFactory() const
{
	return m_WICFactoryPtr;
}

ID2D1RenderTarget* game_engine::GetHwndRenderTarget() const
{
	return m_RenderTargetPtr;
}

IDWriteFactory* game_engine::GetDWriteFactory() const
{
	return m_DWriteFactoryPtr;
}

POINT game_engine::GetMousePosition()const
{
	return m_InputPtr->GetMousePosition();
}
DOUBLE2 game_engine::GetMousePositionDOUBLE2()const
{
    return DOUBLE2(m_InputPtr->GetMousePosition().x,m_InputPtr->GetMousePosition().y);
}

AudioSystem * game_engine::GetXAudio() const
{
	return m_XaudioPtr;
}

void game_engine::set_icon(WORD wIcon)
{
	m_wIcon = wIcon;
}

void game_engine::set_small_icon(WORD wSmallIcon)
{
	m_wSmallIcon = wSmallIcon;
}

void game_engine::set_width(int iWidth)
{
	m_iWidth = iWidth;
}

void game_engine::set_height(int iHeight)
{
	m_iHeight = iHeight;
}

void game_engine::set_physics_step(bool bEnabled)
{
	m_PhysicsStepEnabled = bEnabled;
}

void game_engine::set_sleep(bool bSleep)
{
	if (m_GameTickTimerPtr == nullptr)
		return;

	m_bSleep = bSleep;
	if (bSleep)
	{
		m_GameTickTimerPtr->Stop();
	}
	else
	{
		m_GameTickTimerPtr->Start();
	}
}

void game_engine::enable_vsync(bool bEnable)
{
	m_bVSync = bEnable;
}

void game_engine::GUITick(double deltaTime)
{
	for (GUIBase* guiPtr : m_GUIPtrArr)
	{
		guiPtr->Tick(deltaTime);
	}
}

void game_engine::GUIPaint()
{
	for (GUIBase* guiPtr : m_GUIPtrArr)
	{
		guiPtr->Paint();
	}
}

void game_engine::GUIConsumeEvents()
{
	for (GUIBase* guiPtr : m_GUIPtrArr)
	{
		guiPtr->ConsumeEvent();
	}

}

void game_engine::apply_settings(GameSettings &gameSettings)
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

void game_engine::set_vsync(bool vsync)
{
	m_bVSync = vsync;
}

bool game_engine::get_vsync()
{
	return m_bVSync;
}


// Input methods
bool game_engine::IsKeyboardKeyDown(int key) const
{
	return m_InputPtr->IsKeyboardKeyDown(key);
}

bool game_engine::IsKeyboardKeyPressed(int key) const
{
	return m_InputPtr->IsKeyboardKeyPressed(key);
}

bool game_engine::IsKeyboardKeyReleased(int key) const
{
	return m_InputPtr->IsKeyboardKeyReleased(key);
}

bool game_engine::IsMouseButtonDown(int button) const
{
	return m_InputPtr->IsMouseButtonDown(button);
}

bool game_engine::IsMouseButtonPressed(int button) const
{
	return m_InputPtr->IsMouseButtonPressed(button);
}

bool game_engine::IsMouseButtonReleased(int button) const
{
	return m_InputPtr->IsMouseButtonReleased(button);
}


LRESULT game_engine::handle_event(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
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
		game_engine::instance()->m_bQuit = true;
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
		{
			//If you have changed certain window data using SetWindowLong, you must call SetWindowPos for the changes to take effect.
			SetWindowPos(m_hWindow, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

		}
		RECT r;
		::GetClientRect(m_hWindow, &r);
		m_iWidth = r.right - r.left;
		m_iHeight = r.bottom - r.top;

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
			m_OverlayManager->set_visible(!m_OverlayManager->get_visible());
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
		for (GUIBase* guiPtr : m_GUIPtrArr)
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
void game_engine::CreateDeviceIndependentResources()
{
	// Create Direct3D 11 factory
	{
		CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_DXGIFactoryPtr);

		D3D_FEATURE_LEVEL levels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL featureLevel;
		SUCCEEDED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, &levels, 1, D3D11_SDK_VERSION, &m_D3DDevicePtr, &featureLevel, &m_D3DDeviceContextPtr));
	}

	CreateD2DFactory();
	CreateWICFactory();
	CreateWriteFactory();

	m_D3DDeviceContextPtr->QueryInterface(IID_PPV_ARGS(&m_D3DUserDefinedAnnotation));
}

void game_engine::CreateD2DFactory()
{
	HRESULT hr;
	// Create a Direct2D factory.
	ID2D1Factory* localD2DFactoryPtr = nullptr;
	if (!m_D2DFactoryPtr)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &localD2DFactoryPtr);
		if (FAILED(hr))
		{
			message_box(String("Create D2D Factory Failed"));
			exit(-1);
		}
		m_D2DFactoryPtr = localD2DFactoryPtr;
	}
}

void game_engine::CreateWICFactory()
{
	HRESULT hr;
	// Create a WIC factory if it does not exists
	IWICImagingFactory* localWICFactoryPtr = nullptr;
	if (!m_WICFactoryPtr)
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&localWICFactoryPtr));
		if (FAILED(hr))
		{
			message_box(String("Create WIC Factory Failed"));
			exit(-1);
		}
		m_WICFactoryPtr = localWICFactoryPtr;
	}
}

void game_engine::CreateWriteFactory()
{
	HRESULT hr;
	// Create a DirectWrite factory.
	IDWriteFactory* localDWriteFactoryPtr = nullptr;
	if (!m_DWriteFactoryPtr)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(localDWriteFactoryPtr), reinterpret_cast<IUnknown **>(&localDWriteFactoryPtr));
		if (FAILED(hr))
		{
			message_box(String("Create WRITE Factory Failed"));
			exit(-1);
		}
		m_DWriteFactoryPtr = localDWriteFactoryPtr;
	}
}

//
//  This method creates resources which are bound to a particular
//  Direct3D device. It's all centralized here, in case the resources
//  need to be recreated in case of Direct3D device loss (eg. display
//  change, remoting, removal of video card, etc).
//
void game_engine::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_DXGISwapchainPtr)
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
		if (m_GameSettings.m_FullscreenMode == GameSettings::FullScreenMode::Windowed || m_GameSettings.m_FullscreenMode == GameSettings::FullScreenMode::BorderlessWindowed)
		{
			desc.Windowed = TRUE;
		}
		else
		{
			desc.Windowed = false;
		}
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SUCCEEDED(m_DXGIFactoryPtr->CreateSwapChain(m_D3DDevicePtr, &desc, &m_DXGISwapchainPtr));

		SetDebugName(m_DXGISwapchainPtr, "DXGISwapchain");
		SetDebugName(m_DXGIFactoryPtr, "DXGIFactory");

		ID3D11Texture2D* backBuffer;
		m_DXGISwapchainPtr->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		SetDebugName(backBuffer, "DXGIBackBuffer");
		assert(backBuffer);
		SUCCEEDED(m_D3DDevicePtr->CreateRenderTargetView(backBuffer, NULL, &m_D3DBackBufferView));
		SUCCEEDED(m_D3DDevicePtr->CreateShaderResourceView(backBuffer, NULL, &m_D3DBackBufferSRV));
		backBuffer->Release();

		// Create the game viewport texture
		{
			CD3D11_TEXTURE2D_DESC texture_desc{ DXGI_FORMAT_B8G8R8A8_UNORM, (UINT)4, (UINT)4 };
			texture_desc.Usage = D3D11_USAGE_DEFAULT;
			texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			texture_desc.MipLevels = 1;
			texture_desc.ArraySize = 1;
			SUCCEEDED(m_D3DDevicePtr->CreateTexture2D(&texture_desc, NULL, &_game_output_tex));

			texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			SUCCEEDED(m_D3DDevicePtr->CreateTexture2D(&texture_desc, NULL, &_game_output_depth));

			// Create the views
			SUCCEEDED(m_D3DDevicePtr->CreateDepthStencilView(_game_output_depth, NULL, &_game_output_dsv));
			SUCCEEDED(m_D3DDevicePtr->CreateRenderTargetView(_game_output_tex, NULL, &_game_output_rtv));
			SUCCEEDED(m_D3DDevicePtr->CreateShaderResourceView(_game_output_tex, NULL, &_game_output_srv));
		}


		UINT dpi = GetDpiForWindow(get_window());
		D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);

		IDXGISurface* surface;
		_game_output_tex->QueryInterface(&surface);

		hr = m_D2DFactoryPtr->CreateDxgiSurfaceRenderTarget(surface, rtp, &m_RenderTargetPtr);
		surface->Release();

		if (FAILED(hr))
		{
			message_box(String("Create CreateDeviceResources Failed"));
			exit(-1);
		}

		//set alias mode
		m_RenderTargetPtr->SetAntialiasMode(m_D2DAAMode);

		// Create a brush.
		m_RenderTargetPtr->CreateSolidColorBrush((D2D1::ColorF) D2D1::ColorF::Black, &m_ColorBrushPtr);

		//Create a Font
		m_DefaultFontPtr = new Font(String("Consolas"), 12);
		m_UserFontPtr = m_DefaultFontPtr;
		m_bInitialized = true;
	}
}

//
//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
//
void game_engine::DiscardDeviceResources()
{
	m_bInitialized = false;
	if (m_ColorBrushPtr)
	{
		m_ColorBrushPtr->Release();
		m_ColorBrushPtr = nullptr;
	}
	if (m_RenderTargetPtr)
	{
		m_RenderTargetPtr->Release();
		m_RenderTargetPtr = nullptr;
	}

	m_D3DBackBufferView->Release();
	m_D3DBackBufferView = nullptr;

	m_DXGISwapchainPtr->Release();
	m_DXGISwapchainPtr = nullptr;
}

void game_engine::D2DBeginPaint()
{
	if (m_RenderTargetPtr)
	{
		m_RenderTargetPtr->BeginDraw();
		m_RenderTargetPtr->SetTransform(D2D1::Matrix3x2F::Identity());

		// set black as initial brush color 
		m_ColorBrushPtr->SetColor(D2D1::ColorF((FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(1.0)));
	}
}

bool game_engine::D2DEndPaint()
{
	HRESULT hr = S_OK;
	hr = m_RenderTargetPtr->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET)
	{
		message_box(String(" Direct2D error: RenderTarget lost.\nThe GameEngine terminates the game.\n"));
		return false; //app should close or re-initialize
	}
	return true;
}

// Box2D overloads
void game_engine::BeginContact(b2Contact* contactPtr)
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
            m_BeginContactDataArr.push_back(contactData);
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
            m_BeginContactDataArr.push_back(contactData);
        }
    }
};

void game_engine::EndContact(b2Contact* contactPtr)
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
            m_EndContactDataArr.push_back(contactData);
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
            m_EndContactDataArr.push_back(contactData);
        }
    }
};

void game_engine::PreSolve(b2Contact* contactPtr, const b2Manifold* oldManifoldPtr)
{

}

void game_engine::PostSolve(b2Contact* contactPtr, const b2ContactImpulse* impulsePtr)
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

	if(sum > 0.00001) m_ImpulseDataArr.push_back(impulseData);
}

void game_engine::CallListeners()
{
	// begin contact
	for (size_t i = 0; i < m_BeginContactDataArr.size(); i++)
	{
		ContactListener * contactListenerPtr = reinterpret_cast<ContactListener *>(m_BeginContactDataArr[i].contactListenerPtr);
		contactListenerPtr->BeginContact(
			reinterpret_cast<PhysicsActor *>(m_BeginContactDataArr[i].actThisPtr),
			reinterpret_cast<PhysicsActor *>(m_BeginContactDataArr[i].actOtherPtr)
		);
	}
	m_BeginContactDataArr.clear();

	// end contact
	for (size_t i = 0; i < m_EndContactDataArr.size(); i++)
	{
   
            ContactListener * contactListenerPtr = reinterpret_cast<ContactListener *>(m_EndContactDataArr[i].contactListenerPtr);
            contactListenerPtr->EndContact(
                reinterpret_cast<PhysicsActor *>(m_EndContactDataArr[i].actThisPtr),
                reinterpret_cast<PhysicsActor *>(m_EndContactDataArr[i].actOtherPtr)
                );
		
	}
	m_EndContactDataArr.clear();

	// impulses
	for (size_t i = 0; i < m_ImpulseDataArr.size(); i++)
	{
		ContactListener * contactListenerAPtr = reinterpret_cast<ContactListener *>(m_ImpulseDataArr[i].contactListenerAPtr);
		ContactListener * contactListenerBPtr = reinterpret_cast<ContactListener *>(m_ImpulseDataArr[i].contactListenerBPtr);
		if (contactListenerAPtr != nullptr) contactListenerAPtr->ContactImpulse(reinterpret_cast<PhysicsActor *>(m_ImpulseDataArr[i].actAPtr), m_ImpulseDataArr[i].impulseA);
		if (contactListenerBPtr != nullptr) contactListenerBPtr->ContactImpulse(reinterpret_cast<PhysicsActor *>(m_ImpulseDataArr[i].actBPtr), m_ImpulseDataArr[i].impulseB);
	}
	m_ImpulseDataArr.clear();
}

void game_engine::build_ui()
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
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("About"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Simulation"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::MenuItem("Debug Log");
				ImGui::MenuItem("Viewport");
				ImGui::MenuItem("Scene Hierarchy");
				ImGui::MenuItem("Properties");

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}
}

int game_engine::run_game(HINSTANCE hInstance, int iCmdShow, AbstractGame* game)
{
	//notify user if heap is corrupt
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE const h_dll = LoadLibrary(L"dxgidebug.dll");
	assert(h_dll);
	fPtr const dxgi_get_debug_interface = reinterpret_cast<fPtr>(GetProcAddress(h_dll, "DXGIGetDebugInterface"));
	assert(dxgi_get_debug_interface);

	IDXGIDebug* pDXGIDebug;
	dxgi_get_debug_interface(__uuidof(IDXGIDebug), reinterpret_cast<void**>(&pDXGIDebug));
	//_CrtSetBreakAlloc(13143);
#endif



	int returnValue = 0;
	game_engine::instance()->set_game(game);
	//game_engine::instance()->SetGame(new TestGame());	
	returnValue = game_engine::instance()->run(hInstance, iCmdShow); // run the game engine and return the result

	// Shutdown the game engine to make sure there's no leaks left. 
	game_engine::Shutdown();

#if defined(DEBUG) | defined(_DEBUG)
	if (pDXGIDebug) pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	pDXGIDebug->Release();
#endif

	return returnValue;
}


