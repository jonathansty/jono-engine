#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"


using framework::World;
using framework::EntityHandle;

class HelloWorldGame : public AbstractGame
{
public:

	void GameInitialize(GameSettings& gameSettings)
	{
		gameSettings.m_WindowFlags |= GameSettings::WindowFlags::EnableConsole;
	}
	void GameStart() override;

	void GameEnd() override;

	void GamePaint(RECT rect) override;

	void GameTick(double deltaTime) override;

	void DebugUI() override;

private:
	std::unique_ptr<World> _world;
	EntityHandle _rotatorEntity;
	EntityHandle _parentEntity;
};


