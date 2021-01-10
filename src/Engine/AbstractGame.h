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

	virtual void initialize(GameSettings &gameSettings) { 	UNREFERENCED_PARAMETER(gameSettings); }								// empty definition
	virtual void start(void) {}																// empty definition
	virtual void end(void) {}																// empty definition
	virtual void paint(RECT rect) { UNREFERENCED_PARAMETER(rect); }														// empty definition
	virtual void tick(double deltaTime) { UNREFERENCED_PARAMETER(deltaTime); }													// empty definition
	virtual void debug_ui() {}
	virtual void render_3d() {};
};


