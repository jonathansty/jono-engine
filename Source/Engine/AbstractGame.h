#pragma once

class PhysicsActor;
class GameSettings;

class GameSettings;
struct EngineSettings;

#include "graphics/2DRenderContext.h"

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

	virtual void configure_engine(EngineSettings &){};

	virtual void initialize(GameSettings &gameSettings) { 	UNREFERENCED_PARAMETER(gameSettings); }								// empty definition
	virtual void start(void) {}																// empty definition
	virtual void end(void) {}																// empty definition
	virtual void paint(graphics::D2DRenderContext& ctx) { UNREFERENCED_PARAMETER(ctx); }														// empty definition
	virtual void tick(double deltaTime) { UNREFERENCED_PARAMETER(deltaTime); }													// empty definition
	virtual void debug_ui() {}
};


