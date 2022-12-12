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

	virtual void ConfigureEngine(EngineCfg &){};

	virtual void ConfigureGame(GameCfg &gameSettings) { 	UNREFERENCED_PARAMETER(gameSettings); }								// empty definition

	virtual void OnStartup(void) {}																// empty definition
	virtual void OnShutdown(void) {}																// empty definition
	#if FEATURE_D2D
	virtual void OnPaint2D(Graphics::D2DRenderContext& ctx) { UNREFERENCED_PARAMETER(ctx); }														// empty definition
	#endif
	virtual void OnUpdate(double deltaTime) { UNREFERENCED_PARAMETER(deltaTime); }													// empty definition
	virtual void OnDebugUI() {}
};


