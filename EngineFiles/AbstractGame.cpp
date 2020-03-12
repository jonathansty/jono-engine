//-----------------------------------------------------------------
// AbstractGame Object
// C++ Source - AbstractGame.cpp - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "stdafx.h"		// include file to use the game engine
#include "../Resource.h"	// include file to use resources
#include "AbstractGame.h"

int RunGame(HINSTANCE hInstance,int iCmdShow, AbstractGame* game)
{
	//notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE hDll = LoadLibrary(L"dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	IDXGIDebug* pDXGIDebug;
	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDXGIDebug);
	//_CrtSetBreakAlloc(13143);
#endif



	int returnValue = 0;
	GameEngine::Instance()->SetGame(game);
	//GAME_ENGINE->SetGame(new TestGame());	
	returnValue = GameEngine::Instance()->Run(hInstance, iCmdShow); // run the game engine and return the result

	delete GameEngine::Instance();
	///////////////////////////////////////////////////////


#if defined(DEBUG) | defined(_DEBUG)
	// unresolved external  
	//if (pDXGIDebug) pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	pDXGIDebug->Release();
#endif

	return returnValue;
}
