#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"


class HelloWorldGame : public AbstractGame
{
public:
	void GameInitialize(GameSettings& gameSettings)
	{
		gameSettings.EnableConsole(true);
	}
	void GameStart() override;

	void GameEnd() override;

	void GamePaint(RECT rect) override;

	void GameTick(double deltaTime) override;

	void DebugUI() override;

private:
	std::unique_ptr<framework::World> _world;
	framework::World::EntityId _rotatorEntity;
	framework::World::EntityId _parentEntity;
};


