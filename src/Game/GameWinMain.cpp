//-----------------------------------------------------------------
// Game Engine WinMain Function
// C++ Source - GameWinMain.cpp - version v2_16 jan 2015 
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "stdafx.h"
#include "GameWinMain.h"

//ReportLiveObjects:
//To use any of these GUID values, you must include Initguid.h before you include DXGIDebug.h in your code.
#include <Initguid.h>
#include <dxgidebug.h>

#include "ElectronicJonaJoy.h"	
#include "AbstractGame.h"

#include "cli/CommandLine.h"
	
//-----------------------------------------------------------------
// Windows Functions
//-----------------------------------------------------------------

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
int main(const int argc, const char** argvs)
{

	auto cmd = cli::parse(argvs, argc);
	ElectronicJonaJoy *game = new ElectronicJonaJoy();
	return GameEngine::run_game(NULL,cmd,1,game);
}
