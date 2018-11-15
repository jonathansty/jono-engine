//-----------------------------------------------------------------
// Game File
// C++ Source - $safeprojectname$.cpp - version v2_16 jan 2015
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------
#include "stdafx.h"		// this include must be the first include line of every cpp file (due to using precompiled header)
	
//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "$safeprojectname$.h"																				

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//-----------------------------------------------------------------
// $safeprojectname$ methods																				
//-----------------------------------------------------------------

$safeprojectname$::$safeprojectname$()
{
	// nothing to create
}

$safeprojectname$::~$safeprojectname$()																						
{
	// nothing to destroy
}

void $safeprojectname$::GameInitialize(GameSettings &gameSettings)
{
	gameSettings.SetWindowTitle(String("$safeprojectname$ - Name, First name - group"));
	gameSettings.SetWindowWidth(842);
	gameSettings.SetWindowHeight(480);
	gameSettings.EnableConsole(false);
	gameSettings.EnableAntiAliasing(false);
}

void $safeprojectname$::GameStart()
{
	// Insert the code that needs to be executed at the start of the game
}

void $safeprojectname$::GameEnd()
{
	// Insert the code that needs to be executed at the closing of the game
}

void $safeprojectname$::GameTick(double deltaTime)
{
	// Insert the code that needs to be executed, EXCEPT for paint commands (see next method)
}

void $safeprojectname$::GamePaint(RECT rect)
{

	// Insert the code that needs to be executed each time a new frame needs to be drawn to the screen
	// Technical note: engine uses double buffering when the gamecycle is running

}



