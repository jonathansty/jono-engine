#pragma once

class PhysicsActor;

struct GameCfg;
struct EngineCfg;

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

	virtual void configure_engine(EngineCfg &){};

	virtual void initialize(GameCfg &gameSettings) { 	UNREFERENCED_PARAMETER(gameSettings); }								// empty definition
	virtual void start(void) {}																// empty definition
	virtual void end(void) {}																// empty definition
	#if FEATURE_D2D
	virtual void paint(Graphics::D2DRenderContext& ctx) { UNREFERENCED_PARAMETER(ctx); }														// empty definition
	#endif
	virtual void tick(double deltaTime) { UNREFERENCED_PARAMETER(deltaTime); }													// empty definition
	virtual void debug_ui() {}
};


