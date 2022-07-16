#pragma once

#include "Engine/AbstractGame.h"

class PathFindingGame : public AbstractGame
{

public:
	void configure_engine(EngineSettings&) override;

	void initialize(GameSettings& gameSettings) override;

	void start(void) override;

	void end(void) override;

	void paint(graphics::D2DRenderContext& ctx) override;

	void tick(double deltaTime) override;

	void debug_ui() override;
};