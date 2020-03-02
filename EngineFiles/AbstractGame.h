#pragma once

class PhysicsActor;
class GameSettings;

class AbstractGame
{
public : 	
	AbstractGame() 
	{
	}

	virtual ~AbstractGame() 
	{
	}

	AbstractGame(const AbstractGame&) = delete;
	AbstractGame& operator=(const AbstractGame&) = delete;

	virtual void GameInitialize(GameSettings &gameSettings) {}								// empty definition
	virtual void GameStart(void) {}																// empty definition
	virtual void GameEnd(void) {}																// empty definition
	virtual void GamePaint(RECT rect) {}														// empty definition
	virtual void GameTick(double deltaTime) {}													// empty definition
	virtual void DebugUI() {}
	virtual void Render3D() {};
};

int RunGame(HINSTANCE hInstance, int iCmdShow, AbstractGame* game);

