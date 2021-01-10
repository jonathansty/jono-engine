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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	auto cmd = cli::parse(szCmdLine);
	bool lvl_arg = cli::has_arg(cmd, "-level");
	if(lvl_arg) {
		std::cout << "Booting custom level. \n";
		std::string lvl_value;
		cli::get_string(cmd, "-level", lvl_value);
		std::cout << "Level: " << lvl_value << std::endl;
	}

	int max_fps = 0;
	if(cli::get_number(cmd, "-max-fps",max_fps)) {
		std::cout << "Max FPS set to " << max_fps << std::endl;
	}


	return game_engine::run_game(hInstance,iCmdShow, new ElectronicJonaJoy());
}
