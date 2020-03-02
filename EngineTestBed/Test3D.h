#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"

class Hello3D : public AbstractGame
{
public:
	void GameInitialize(GameSettings& gameSettings);
	void GameStart() override;
	void GameEnd() override;
	void GamePaint(RECT rect) override;
	void GameTick(double deltaTime) override;
	void DebugUI() override;
	void Render3D() override;

private:
	std::unique_ptr<framework::World> _world;

	ComPtr<ID3D11Buffer> _cb_MVP;

};


