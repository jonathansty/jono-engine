#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"


using framework::World;
using framework::EntityHandle;

class HelloWorldGame : public AbstractGame
{
public:
	HelloWorldGame() = default;
	virtual ~HelloWorldGame() = default;

	void configure_engine(EngineSettings& engineSettings) {
		engineSettings.d2d_use = true;
		engineSettings.d2d_use_aa = true;
	}
	void initialize(GameSettings& gameSettings) {}

	void start() override;

	void end() override;

	void paint(graphics::D2DRenderContext& ctx) override;

	void tick(double deltaTime) override;

	void debug_ui() override;

private:
	std::shared_ptr<World> _world;
	EntityHandle _rotatorEntity;
	EntityHandle _parentEntity;
};


