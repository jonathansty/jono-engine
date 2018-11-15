//-----------------------------------------------------------------
// Game File
// C++ Source - $safeprojectname$.h - version v2_16 jan 2015
// Copyright DAE Programming Team
// http://www.digitalartsandentertainment.be/
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Student data
// Name:
// Group:
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------

#include "Resource.h"	
#include "AbstractGame.h"

//-----------------------------------------------------------------
// $safeprojectname$ Class																
//-----------------------------------------------------------------
class $safeprojectname$ : public AbstractGame
{
public:				
	//---------------------------
	// Constructor(s)
	//---------------------------
	$safeprojectname$();

	//---------------------------
	// Destructor
	//---------------------------
	virtual ~$safeprojectname$();

	// C++11 make the class non-copyable
	$safeprojectname$(const $safeprojectname$&) = delete;
	$safeprojectname$& operator=(const $safeprojectname$&) = delete;

	//---------------------------
	// General Methods
	//---------------------------

	virtual void GameInitialize(GameSettings &gameSettings);
	virtual void GameStart();				
	virtual void GameEnd();
	virtual void GameTick(double deltaTime);
	virtual void GamePaint(RECT rect);

	// -------------------------
	// Public Member functions
	// -------------------------

private:
	// -------------------------
	// Private Member functions
	// -------------------------

	// -------------------------
	// Private Datamembers
	// -------------------------

};
