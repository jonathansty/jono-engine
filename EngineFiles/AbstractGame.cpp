#include "stdafx.h"	
#include "AbstractGame.h"

int RunGame(HINSTANCE hInstance,int iCmdShow, AbstractGame* game)
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
	GameEngine::Instance()->SetGame(game);
	//GAME_ENGINE->SetGame(new TestGame());	
	returnValue = GameEngine::Instance()->Run(hInstance, iCmdShow); // run the game engine and return the result

	// Shutdown the game engine to make sure there's no leaks left. 
	GameEngine::Shutdown();

#if defined(DEBUG) | defined(_DEBUG)
	if (pDXGIDebug) pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	pDXGIDebug->Release();
#endif

	return returnValue;
}
