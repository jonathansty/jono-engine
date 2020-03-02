#include "stdafx.h"
#include "../stdafx.h"

#include "GameEngine.h"
#include "ContactListener.h"
#include "AbstractGame.h"

#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "DebugOverlays/MetricsOverlay.h"
#include "DebugOverlays/RTTIDebugOverlay.h"
#include "DebugOverlays/ImGuiOverlays.h"

//-----------------------------------------------------------------
// Windows Functions
//-----------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route all Windows messages to the game engine
	return GameEngine::GetSingleton()->HandleEvent(hWindow, msg, wParam, lParam);
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
//-----------------------------------------------------------------
// GameEngine Constructor(s)/Destructor
//-----------------------------------------------------------------
GameEngine::GameEngine() : 
	m_hInstance(0),
	m_hWindow(NULL),
	m_bSleep(true),
	m_bPaintingAllowed(false), //changed in june 2014, reset to false in dec 2014
	m_iWidth(0), m_iHeight(0),
	m_wSmallIcon(0), m_wIcon(0), m_dKeybThreadID(0),
	m_GamePtr(nullptr),
	m_D2DFactoryPtr(nullptr),
	m_WICFactoryPtr(nullptr),
	m_RenderTargetPtr(nullptr),
	m_DWriteFactoryPtr(nullptr),
	m_GameTickTimerPtr(nullptr),
	m_bInitialized(false),
	m_ColorBrushPtr(nullptr),
	m_AntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED),
	m_BitmapInterpolationMode(D2D1_BITMAP_INTERPOLATION_MODE_LINEAR),
	m_DefaultFontPtr(nullptr),
	m_UserFontPtr(nullptr),
	m_ConsoleHandle(NULL),
	m_bVSync(true),
	m_InputPtr(nullptr),
	m_XaudioPtr(nullptr),
	m_D3DDevicePtr(nullptr),
	m_D3DDeviceContextPtr(nullptr),
	m_DXGIFactoryPtr(nullptr),
	m_DXGISwapchainPtr(nullptr)
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

GameEngine::~GameEngine()
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

//-----------------------------------------------------------------
// Game Engine Static Methods
//-----------------------------------------------------------------
GameEngine* GameEngine::GetSingleton()
{
	return GameEngine::Instance();
}

void GameEngine::SetGame(AbstractGame* gamePtr)
{
	m_GamePtr = gamePtr;
}

//-----------------------------------------------------------------
// Game Engine General Methods
//-----------------------------------------------------------------
void GameEngine::SetTitle(const String& titleRef)
{
	m_Title = titleRef;
}

int GameEngine::Run(HINSTANCE hInstance, int iCmdShow)
{
	// create the game engine object, exit if failure
	if (GameEngine::GetSingleton() == nullptr) return false;

	// set the instance member variable of the game engine
	GameEngine::GetSingleton()->SetInstance(hInstance);

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
	GameSettings gameSettings;
	m_GamePtr->GameInitialize(gameSettings);
	ApplyGameSettings(gameSettings);

	// Open the window
	if (!GameEngine::GetSingleton()->RegisterWindowClass())
	{
		MessageBoxA(NULL, "Register class failed", "error", MB_OK);
		return false;
	}
	if (!GameEngine::GetSingleton()->OpenWindow(iCmdShow))
	{
		MessageBoxA(NULL, "Open window failed", "error", MB_OK);
		return false;
	}


	// Initialize the Graphics Engine
	CreateDeviceResources();

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(GetWindow());
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
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			//std::cout << timesum / count << '\n';
			//OutputDebugString(String("\nav sleeptime: ") + String(timesum/count,8) );
			//timesum = 0;
			//count = 0;
		}
		else
		{
			// Make sure the game engine isn't sleeping
			if (m_bSleep == false)
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
					m_Box2DWorldPtr->Step((float)m_PhysicsTimeStep, velocityIterations, positionIterations);

					// Step generates contact lists, pass to Listeners and clear the vector
					CallListeners();
					lag -= m_PhysicsTimeStep;
				}
				t.Stop();
				m_MetricsOverlay->UpdateTimer(MetricsOverlay::Timer::GameUpdateCPU, (float)t.GetTimeInMS());


				ImGui_ImplDX11_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				{
					m_GamePtr->DebugUI();
					m_OverlayManager->render_overlay();
				}
				ImGui::EndFrame();
				ImGui::Render();

				// Get pgu data 
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




				{
					GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"Frame");

					m_D3DDeviceContextPtr->Begin(gpuTimings[idx][0]);
					m_D3DDeviceContextPtr->End(gpuTimings[idx][1]);



					D3D11_VIEWPORT vp{};
					vp.Width = this->GetWidth();
					vp.Height = this->GetHeight();
					vp.TopLeftX = 0.0f;
					vp.TopLeftY = 0.0f;
					vp.MinDepth = 0.0f;
					vp.MaxDepth = 1.0f;
					m_D3DDeviceContextPtr->RSSetViewports(1, &vp);

					FLOAT color[4] = { 0.0f,0.0f,0.0f,0.0f };
					m_D3DDeviceContextPtr->ClearRenderTargetView(m_D3DBackBufferView, color);
					m_D3DDeviceContextPtr->OMSetRenderTargets(1, &m_D3DBackBufferView, nullptr);

					// First render the 3D scene
					{
						GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"Render3D");
						m_GamePtr->Render3D();
					}

					// Paint using vsynch
					ExecuteDirect2DPaint();


					{
						GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"ImGui");
						ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
					}

					// Present
					GPU_MARKER(m_D3DUserDefinedAnnotation, L"DrawEnd");
					m_DXGISwapchainPtr->Present(m_bVSync ? 1 : 0, 0);

					m_D3DDeviceContextPtr->End(gpuTimings[idx][2]);
					m_D3DDeviceContextPtr->End(gpuTimings[idx][0]);
				}
			}
			else WaitMessage(); // if the engine is sleeping or the game loop isn't supposed to run, wait for the next windows message.
		}
	}
	// undo the timer setting
	timeEndPeriod(tc.wPeriodMin);


	// User defined code for exiting the game
	m_GamePtr->GameEnd();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	// Box2D
	delete m_Box2DWorldPtr;

	return static_cast<int>(msg.wParam);
}

void GameEngine::ExecuteDirect2DPaint()
{
	GPU_SCOPED_EVENT(m_D3DUserDefinedAnnotation, L"Game2D");

	D2DBeginPaint();
	RECT usedClientRect = { 0, 0, GetWidth(), GetHeight() };

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
		SetColor(COLOR(0, 0, 0, 127));
		FillRect(0, 0, GetWidth(), GetHeight());
		SetViewMatrix(matView);
		m_Box2DWorldPtr->DrawDebugData();
	}

	// deactivate all gui objects
	GUIConsumeEvents();

	m_bPaintingAllowed = false;
	bool result = D2DEndPaint();

	// if drawing failed, terminate the game
	if (!result) PostMessage(GameEngine::GetWindow(), WM_DESTROY, 0, 0);
}

bool GameEngine::RegisterWindowClass()
{
	WNDCLASSEX wndclass;

	// Create the window class for the main window
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = m_hInstance;
	wndclass.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(GetIcon()));
	wndclass.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(GetSmallIcon()));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = m_Title.C_str();

	// Register the window class
	if (!RegisterClassEx(&wndclass)) return false;
	return true;
}

bool GameEngine::OpenWindow(int iCmdShow)
{
	// Calculate the window size and position based upon the game size
	DWORD windowStyle = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
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
	ShowWindow(m_hWindow, iCmdShow);
	UpdateWindow(m_hWindow);

	return true;
}

void GameEngine::QuitGame()
{
	PostMessage(GameEngine::GetWindow(), WM_DESTROY, 0, 0);
}

void GameEngine::MessageBox(const String &text) const
{
	if (sizeof(TCHAR) == 2)	::MessageBoxW(GetWindow(),(wchar_t*)text.C_str(), (wchar_t*)m_Title.C_str(), MB_ICONEXCLAMATION | MB_OK);
	else MessageBoxA(GetWindow(), (char*)text.C_str(), (char*)m_Title.C_str(), MB_ICONEXCLAMATION | MB_OK);
}

void GameEngine::ConsoleCreate()
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

void GameEngine::ConsoleSetForeColor(bool red, bool green, bool blue, bool intensity)
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

void GameEngine::ConsoleSetBackColor(bool red, bool green, bool blue, bool intensity)
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

void GameEngine::ConsoleSetCursorPosition(int column, int row)
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

void GameEngine::ConsolePrintString(const String& textRef)
{
#ifdef _UNICODE
	std::wcout << textRef.C_str() << "\n";
#else
	std::cout << textRef.C_str() << "\n";
#endif
	String copy = textRef + String("\n");
	::OutputDebugString(copy);
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

void GameEngine::ConsolePrintString(const String& textRef, int column, int row)
{
	ConsoleSetCursorPosition(column, row);
	ConsolePrintString(textRef);
}

//void GameEngine::ConsolePrintString(string text, int column, int row) 
//{
//	ConsoleSetCursorPosition(column,  row);
//	ConsolePrintString(text);
//}

void GameEngine::ConsoleClear() const
{
	system("cls");
}

void GameEngine::SetInstance(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}

void GameEngine::SetWindow(HWND hWindow)
{
	m_hWindow = hWindow;
}

bool GameEngine::CanIPaint() const
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

void GameEngine::SetColor(COLOR color)
{
	m_ColorBrushPtr->SetColor(D2D1::ColorF((FLOAT)(color.red / 255.0), (FLOAT)(color.green / 255.0), (FLOAT)(color.blue / 255.0), (FLOAT)(color.alpha / 255.0)));
}

COLOR GameEngine::GetColor()
{
	D2D1_COLOR_F dColor = m_ColorBrushPtr->GetColor();
	return COLOR((unsigned char)(dColor.r * 255), (unsigned char)(dColor.g * 255), (unsigned char)(dColor.b * 255), (unsigned char)(dColor.a * 255));
}

bool GameEngine::DrawSolidBackground(COLOR backgroundColor)
{
	if (!CanIPaint()) return false;

	m_RenderTargetPtr->Clear(D2D1::ColorF((FLOAT)(backgroundColor.red / 255.0), (FLOAT)(backgroundColor.green / 255.0), (FLOAT)(backgroundColor.blue / 255.0), (FLOAT)(backgroundColor.alpha)));

	return true;
}

bool GameEngine::DrawLine(int x1, int y1, int x2, int y2)
{
	return DrawLine(DOUBLE2(x1, y1), DOUBLE2(x2, y2), 1.0);
}

bool GameEngine::DrawLine(DOUBLE2 p1, DOUBLE2 p2, double strokeWidth)
{
	if (!CanIPaint()) return false;
	m_RenderTargetPtr->DrawLine(Point2F((FLOAT)p1.x, (FLOAT)p1.y), Point2F((FLOAT)p2.x, (FLOAT)p2.y), m_ColorBrushPtr, (FLOAT)strokeWidth);

	return true;
}

bool GameEngine::DrawPolygon(const std::vector<POINT>& ptsArr, unsigned int count, bool close)
{
	if (!CanIPaint()) return false;
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
	if (!CanIPaint()) return false;
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
	if (!CanIPaint()) return false;
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
		GameEngine::GetSingleton()->MessageBox(String("Failed to create path geometry"));
		return false;
	}

	// Write to the path geometry using the geometry sink.
	ID2D1GeometrySink* geometrySinkPtr = nullptr;
	hr = geometryPtr->Open(&geometrySinkPtr);
	if (FAILED(hr))
	{
		geometrySinkPtr->Release();
		geometryPtr->Release();
		GameEngine::GetSingleton()->MessageBox(String("Failed to open path geometry"));
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

bool GameEngine::FillPolygon(const std::vector<DOUBLE2>& ptsArr, unsigned int count)
{
	if (!CanIPaint())return false;
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
		GameEngine::GetSingleton()->MessageBox(String("Failed to create path geometry"));
		return false;
	}

	// Write to the path geometry using the geometry sink.
	ID2D1GeometrySink* geometrySinkPtr = nullptr;
	hr = geometryPtr->Open(&geometrySinkPtr);
	if (FAILED(hr))
	{
		geometrySinkPtr->Release();
		geometryPtr->Release();
		GameEngine::GetSingleton()->MessageBox(String("Failed to open path geometry"));
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
	if (!CanIPaint()) return false;
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) MessageBoxA(NULL, "GameEngine::DrawRect error: invalid dimensions!", "GameEngine says NO", MB_OK);
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	m_RenderTargetPtr->DrawRectangle(d2dRect, m_ColorBrushPtr, (FLOAT)strokeWidth);

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
	if (!CanIPaint()) return false;
	if ((rect.right < rect.left) || (rect.bottom < rect.top)) MessageBoxA(NULL, "GameEngine::DrawRect error: invalid dimensions!", "GameEngine says NO", MB_OK);

	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	m_RenderTargetPtr->FillRectangle(d2dRect, m_ColorBrushPtr);

	return true;
}

bool GameEngine::DrawRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, 1.0);
	return true;
}

bool GameEngine::DrawRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY, double strokeWidth)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, (FLOAT)strokeWidth);
	return true;
}

bool GameEngine::DrawRoundedRect(RECT rect, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, 1.0);
	return true;
}

bool GameEngine::DrawRoundedRect(RECT2 rect, int radiusX, int radiusY, double strokeWidth)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr, (FLOAT)strokeWidth);
	return true;
}

bool GameEngine::FillRoundedRect(int left, int top, int right, int bottom, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)left, (FLOAT)top, (FLOAT)(right), (FLOAT)(bottom));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool GameEngine::FillRoundedRect(DOUBLE2 topLeft, DOUBLE2 rightbottom, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)topLeft.x, (FLOAT)topLeft.y, (FLOAT)(rightbottom.x), (FLOAT)(rightbottom.y));
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool GameEngine::FillRoundedRect(RECT rect, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool GameEngine::FillRoundedRect(RECT2 rect, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;
	D2D1_RECT_F d2dRect = D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom);
	D2D1_ROUNDED_RECT  d2dRoundedRect = D2D1::RoundedRect(d2dRect, (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillRoundedRectangle(d2dRoundedRect, m_ColorBrushPtr);
	return true;
}

bool GameEngine::DrawEllipse(int centerX, int centerY, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawEllipse(ellipse, m_ColorBrushPtr, 1.0);

	return true;
}

bool GameEngine::DrawEllipse(DOUBLE2 centerPt, double radiusX, double radiusY, double strokeWidth)
{
	if (!CanIPaint()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->DrawEllipse(ellipse, m_ColorBrushPtr, (FLOAT)strokeWidth);

	return true;
}

bool GameEngine::FillEllipse(int centerX, int centerY, int radiusX, int radiusY)
{
	if (!CanIPaint()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerX, (FLOAT)centerY), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillEllipse(ellipse, m_ColorBrushPtr);

	return true;
}

bool GameEngine::FillEllipse(DOUBLE2 centerPt, double radiusX, double radiusY)
{
	if (!CanIPaint()) return false;

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F((FLOAT)centerPt.x, (FLOAT)centerPt.y), (FLOAT)radiusX, (FLOAT)radiusY);
	m_RenderTargetPtr->FillEllipse(ellipse, m_ColorBrushPtr);

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
	if (!CanIPaint()) return false;

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
	if (!CanIPaint()) return false;
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

	m_RenderTargetPtr->DrawBitmap(imagePtr->GetBitmapPtr(), dstRect_f, (FLOAT)imagePtr->GetOpacity(), m_BitmapInterpolationMode, srcRect_f);

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
void GameEngine::SetWorldMatrix(const MATRIX3X2& mat)
{
	m_MatWorld = mat;
	D2D1::Matrix3x2F matDirect2D = (m_MatWorld * m_MatView).ToMatrix3x2F();
	m_RenderTargetPtr->SetTransform(matDirect2D);
}

MATRIX3X2 GameEngine::GetWorldMatrix()
{
	return m_MatWorld;
}

//view matrix operations
void GameEngine::SetViewMatrix(const MATRIX3X2& mat)
{
	m_MatView = mat;
	D2D1::Matrix3x2F matDirect2D = (m_MatWorld * m_MatView).ToMatrix3x2F();
	m_RenderTargetPtr->SetTransform(matDirect2D);
}

MATRIX3X2 GameEngine::GetViewMatrix()
{
	return m_MatView;
}

void GameEngine::EnableAntiAlias(bool isEnabled)
{
	if (isEnabled)	m_AntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
	else			m_AntialiasMode = D2D1_ANTIALIAS_MODE_ALIASED;
	if (m_RenderTargetPtr)m_RenderTargetPtr->SetAntialiasMode(m_AntialiasMode);
}

void GameEngine::EnablePhysicsDebugRendering(bool isEnabled)
{
	m_DebugRendering = isEnabled;
}

void GameEngine::SetBitmapInterpolationModeLinear()
{
	m_BitmapInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
}

void GameEngine::SetBitmapInterpolationModeNearestNeighbor()
{
	m_BitmapInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
}

void GameEngine::SetFont(Font* fontPtr)
{
	m_UserFontPtr = fontPtr;
}

Font* GameEngine::GetFont()
{
	return m_UserFontPtr;
}

void GameEngine::SetDefaultFont()
{
	m_UserFontPtr = m_DefaultFontPtr;
}

void GameEngine::RegisterGUI(GUIBase *guiPtr)
{
	m_GUIPtrArr.push_back(guiPtr);
}

void GameEngine::UnRegisterGUI(GUIBase *targetPtr)
{
	std::vector<GUIBase*>::iterator pos = find(m_GUIPtrArr.begin(), m_GUIPtrArr.end(), targetPtr); // find algorithm from STL

	if (pos == m_GUIPtrArr.end()) return;
	else
	{
		m_GUIPtrArr.erase(pos);
		return;
	}
}

HINSTANCE GameEngine::GetInstance() const
{
	return m_hInstance;
}

HWND GameEngine::GetWindow() const
{
	return m_hWindow;
}

String GameEngine::GetTitle() const
{
	//return *m_Title; 
	return m_Title;
}

WORD GameEngine::GetIcon() const
{
	return m_wIcon;
}

WORD GameEngine::GetSmallIcon() const
{
	return m_wSmallIcon;
}

int GameEngine::GetWidth() const
{
	return m_iWidth;
}

int GameEngine::GetHeight() const
{
	return m_iHeight;
}

bool GameEngine::GetSleep() const
{
	return m_bSleep ? true : false;
}

ID3D11Device* GameEngine::GetD3DDevice() const
{
	return m_D3DDevicePtr;
}

ID3D11DeviceContext* GameEngine::GetD3DDeviceContext() const
{
	return m_D3DDeviceContextPtr;
}

ID3D11RenderTargetView* GameEngine::GetD3DBackBufferView() const
{
	return m_D3DBackBufferView;
}

ID2D1Factory* GameEngine::GetD2DFactory() const
{
	return m_D2DFactoryPtr;
}

IWICImagingFactory* GameEngine::GetWICImagingFactory() const
{
	return m_WICFactoryPtr;
}

ID2D1RenderTarget* GameEngine::GetHwndRenderTarget() const
{
	return m_RenderTargetPtr;
}

IDWriteFactory* GameEngine::GetDWriteFactory() const
{
	return m_DWriteFactoryPtr;
}

POINT GameEngine::GetMousePosition()const
{
	return m_InputPtr->GetMousePosition();
}
DOUBLE2 GameEngine::GetMousePositionDOUBLE2()const
{
    return DOUBLE2(m_InputPtr->GetMousePosition().x,m_InputPtr->GetMousePosition().y);
}

AudioSystem * GameEngine::GetXAudio() const
{
	return m_XaudioPtr;
}

void GameEngine::SetIcon(WORD wIcon)
{
	m_wIcon = wIcon;
}

void GameEngine::SetSmallIcon(WORD wSmallIcon)
{
	m_wSmallIcon = wSmallIcon;
}

void GameEngine::SetWidth(int iWidth)
{
	m_iWidth = iWidth;
}

void GameEngine::SetHeight(int iHeight)
{
	m_iHeight = iHeight;
}

void GameEngine::SetSleep(bool bSleep)
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

void GameEngine::EnableVSync(bool bEnable)
{
	m_bVSync = bEnable;
}

void GameEngine::GUITick(double deltaTime)
{
	for (GUIBase* guiPtr : m_GUIPtrArr)
	{
		guiPtr->Tick(deltaTime);
	}
}

void GameEngine::GUIPaint()
{
	for (GUIBase* guiPtr : m_GUIPtrArr)
	{
		guiPtr->Paint();
	}
}

void GameEngine::GUIConsumeEvents()
{
	for (GUIBase* guiPtr : m_GUIPtrArr)
	{
		guiPtr->ConsumeEvent();
	}

}

void GameEngine::ApplyGameSettings(GameSettings &gameSettings)
{
	SetWidth(gameSettings.GetWindowWidth());
	SetHeight(gameSettings.GetWindowHeight());
	SetTitle(gameSettings.GetWindowTitle());
	EnableVSync(gameSettings.IsVSync());
	EnableAntiAlias(gameSettings.IsAntiAliasingEnabled());

	if (gameSettings.IsConsoleEnabled())
	{
		ConsoleCreate();
	}
}

void GameEngine::SetVSync(bool vsync)
{
	m_bVSync = vsync;
}

bool GameEngine::GetVSync()
{
	return m_bVSync;
}

// Input methods
bool GameEngine::IsKeyboardKeyDown(int key) const
{
	return m_InputPtr->IsKeyboardKeyDown(key);
}

bool GameEngine::IsKeyboardKeyPressed(int key) const
{
	return m_InputPtr->IsKeyboardKeyPressed(key);
}

bool GameEngine::IsKeyboardKeyReleased(int key) const
{
	return m_InputPtr->IsKeyboardKeyReleased(key);
}

bool GameEngine::IsMouseButtonDown(int button) const
{
	return m_InputPtr->IsMouseButtonDown(button);
}

bool GameEngine::IsMouseButtonPressed(int button) const
{
	return m_InputPtr->IsMouseButtonPressed(button);
}

bool GameEngine::IsMouseButtonReleased(int button) const
{
	return m_InputPtr->IsMouseButtonReleased(button);
}


LRESULT GameEngine::HandleEvent(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC         hDC;
	PAINTSTRUCT ps;

	// Get window rectangle and HDC
	RECT windowClientRect;
	GetClientRect(hWindow, &windowClientRect);

	RECT usedClientRect;
	usedClientRect.left = 0;
	usedClientRect.top = 0;
	usedClientRect.right = GetWidth();
	usedClientRect.bottom = GetHeight();

	// Route Windows messages to game engine member functions
	switch (msg)
	{
	case WM_CREATE:
		// Set the game window 
		SetWindow(hWindow);

		return 0;

	case WM_ACTIVATE:
		// Activate/deactivate the game and update the Sleep status
		if (wParam != WA_INACTIVE)
		{
			//Lock hDC
			hDC = GetDC(hWindow);

			// Release HDC
			ReleaseDC(hWindow, hDC);

			SetSleep(false);
		}
		else
		{
			//Lock hDC
			hDC = GetDC(hWindow);

			// Release HDC
			ReleaseDC(hWindow, hDC);

			SetSleep(true);
		}
		return 0;

		//when the window is dragged around, stop the timer
	case WM_ENTERSIZEMOVE:
		SetSleep(true);
		return 0;

	case WM_EXITSIZEMOVE:
		SetSleep(false);
		return 0;

	case WM_PAINT:
		//WM_PAINT needs BeginPaint and EndPaint
		hDC = BeginPaint(hWindow, &ps);
		if (m_bInitialized == true) ExecuteDirect2DPaint();
		EndPaint(hWindow, &ps);
		return 0;

		//case WM_CTLCOLOREDIT:
		//	return SendMessage((HWND) lParam, WM_CTLCOLOREDIT, wParam, lParam);	// delegate this message to the child window

		//case WM_CTLCOLORBTN:
		//	return SendMessage((HWND) lParam, WM_CTLCOLOREDIT, wParam, lParam);	// delegate this message to the child window

		//case WM_LBUTTONDOWN:
		//	m_GamePtr->MouseButtonAction(true, true, (int) LOWORD(lParam), (int) HIWORD(lParam), wParam);
		//	return 0;

		//case WM_LBUTTONUP:
		//	m_GamePtr->MouseButtonAction(true, false, (int) LOWORD(lParam), (int) HIWORD(lParam), wParam);
		//	return 0;

		//case WM_RBUTTONDOWN:
		//	m_GamePtr->MouseButtonAction(false, true, (int) LOWORD(lParam), (int) HIWORD(lParam), wParam);
		//	return 0;

		//case WM_RBUTTONUP:
		//	m_GamePtr->MouseButtonAction(false, false, (int) LOWORD(lParam), (int) HIWORD(lParam), wParam);
		//	return 0;

		//case WM_MOUSEMOVE:
		//	//Store pos for GetMousePosition()
		//	m_MousePosition.x = (int) LOWORD(lParam);
		//	m_MousePosition.y = (int) HIWORD(lParam);

		//	//Pass the Position to the Game Function
		//	m_GamePtr->MouseMove((int) LOWORD(lParam), (int) HIWORD(lParam), wParam);
		//	return 0;

	case WM_SYSCOMMAND:	// trapping this message prevents a freeze after the ALT key is released
		if (wParam == SC_KEYMENU) return 0;			// see win32 API : WM_KEYDOWN
		else break;

	case WM_DESTROY:
		// End the game and exit the application
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
		{
			// switch off the title bar
			DWORD dwStyle = GetWindowLong(m_hWindow, GWL_STYLE);
			dwStyle &= ~WS_CAPTION;
			SetWindowLong(m_hWindow, GWL_STYLE, dwStyle);
			//If you have changed certain window data using SetWindowLong, you must call SetWindowPos for the changes to take effect.
			SetWindowPos(m_hWindow, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			return 0;
		}
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
//
void GameEngine::CreateDeviceIndependentResources()
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

void GameEngine::CreateD2DFactory()
{
	HRESULT hr;
	// Create a Direct2D factory.
	ID2D1Factory* localD2DFactoryPtr = nullptr;
	if (!m_D2DFactoryPtr)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &localD2DFactoryPtr);
		if (FAILED(hr))
		{
			MessageBox(String("Create D2D Factory Failed"));
			exit(-1);
		}
		m_D2DFactoryPtr = localD2DFactoryPtr;
	}
}

void GameEngine::CreateWICFactory()
{
	HRESULT hr;
	// Create a WIC factory if it does not exists
	IWICImagingFactory* localWICFactoryPtr = nullptr;
	if (!m_WICFactoryPtr)
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&localWICFactoryPtr));
		if (FAILED(hr))
		{
			MessageBox(String("Create WIC Factory Failed"));
			exit(-1);
		}
		m_WICFactoryPtr = localWICFactoryPtr;
	}
}

void GameEngine::CreateWriteFactory()
{
	HRESULT hr;
	// Create a DirectWrite factory.
	IDWriteFactory* localDWriteFactoryPtr = nullptr;
	if (!m_DWriteFactoryPtr)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(localDWriteFactoryPtr), reinterpret_cast<IUnknown **>(&localDWriteFactoryPtr));
		if (FAILED(hr))
		{
			MessageBox(String("Create WRITE Factory Failed"));
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
void GameEngine::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_DXGISwapchainPtr)
	{
		DXGI_SWAP_CHAIN_DESC desc{};
		desc.BufferDesc.Width = GetWidth();
		desc.BufferDesc.Height = GetHeight();
		desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.OutputWindow = GetWindow();
		desc.Windowed = TRUE;
		desc.BufferCount = 2;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SUCCEEDED(m_DXGIFactoryPtr->CreateSwapChain(m_D3DDevicePtr, &desc, &m_DXGISwapchainPtr));

		SetDebugName(m_DXGISwapchainPtr, "DXGISwapchain");
		SetDebugName(m_DXGIFactoryPtr, "DXGIFactory");


		ID3D11Texture2D* backBuffer;
		m_DXGISwapchainPtr->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		SetDebugName(backBuffer, "DXGIBackBuffer");
		assert(backBuffer);
		m_D3DDevicePtr->CreateRenderTargetView(backBuffer, NULL, &m_D3DBackBufferView);
		m_D3DDeviceContextPtr->OMSetRenderTargets(1, &m_D3DBackBufferView, NULL);

		// Create a Direct2D render target.
		// EndPaint waits till VBLank !!! when OPTIONS_NONE
		//use D2D1_PRESENT_OPTIONS::D2D1_PRESENT_OPTIONS_NONE for vblank sync
		//and D2D1_PRESENT_OPTIONS::D2D1_PRESENT_OPTIONS_IMMEDIATELY for no waiting
		//D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS   
		D2D1_PRESENT_OPTIONS pres_opt;
		if(m_bVSync)
		{
			//wait for vertical blanking
			pres_opt = D2D1_PRESENT_OPTIONS_NONE;
		}
		else
		{
			//do NOT wait for vertical blanking
			pres_opt = D2D1_PRESENT_OPTIONS_IMMEDIATELY;
		}

		UINT dpi = GetDpiForWindow(GetWindow());
		D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), (FLOAT)dpi, (FLOAT)dpi);

		IDXGISurface* dxgiBackBuffer;
		m_DXGISwapchainPtr->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
		assert(dxgiBackBuffer);
		hr = m_D2DFactoryPtr->CreateDxgiSurfaceRenderTarget(dxgiBackBuffer, rtp, &m_RenderTargetPtr);

		dxgiBackBuffer->Release();

		if (FAILED(hr))
		{
			MessageBox(String("Create CreateDeviceResources Failed"));
			exit(-1);
		}

		//set alias mode
		m_RenderTargetPtr->SetAntialiasMode(m_AntialiasMode);

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
void GameEngine::DiscardDeviceResources()
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

void GameEngine::D2DBeginPaint()
{
	if (m_RenderTargetPtr)
	{
		m_RenderTargetPtr->BeginDraw();
		m_RenderTargetPtr->SetTransform(D2D1::Matrix3x2F::Identity());

		// set black as initial brush color 
		m_ColorBrushPtr->SetColor(D2D1::ColorF((FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(0.0), (FLOAT)(1.0)));
	}
}

bool GameEngine::D2DEndPaint()
{
	HRESULT hr = S_OK;
	hr = m_RenderTargetPtr->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET)
	{
		MessageBox(String(" Direct2D error: RenderTarget lost.\nThe GameEngine terminates the game.\n"));
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

	if(sum > 0.00001) m_ImpulseDataArr.push_back(impulseData);
}

void GameEngine::CallListeners()
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

