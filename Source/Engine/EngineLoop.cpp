#include "engine.pch.h"
#include "EngineLoop.h"
#include "GameEngine.h"

REGISTER_TYPE("/Types/EngineLoop", EngineLoop);
REGISTER_TYPE("/Types/EditorLoop", EditorLoop);

SERIALIZE_FN(EngineLoop) {}
SERIALIZE_FN(EditorLoop) {}

int EngineLoop::Run()
{
	MemoryTracker::init();

	Startup();

	while (m_Engine->m_IsRunning)
	{
		Update(0.033);
	}

	Shutdown();

	return 0;
}

void EngineLoop::Startup()
{
	GameEngine::create();
	m_Engine = GameEngine::instance();

	//#TODO: Should we have some kind of base game? Or drive the game modes in a different way with engine features?
	m_Engine->Startup();

}

void EngineLoop::Update(f64 dt)
{
	m_Engine->Update(dt);
}

void EngineLoop::Shutdown()
{
	m_Engine->Shutdown();
}

void EditorLoop::Update(f64 dt)
{
	// #TODO: Implement pre-update

	Super::Update(dt);

	// #TODO: Implement post-update 
}
