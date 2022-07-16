#include "PathFindingTest.pch.h"
#include "PathFindingGame.h"

void PathFindingGame::configure_engine(EngineSettings& settings)
{
	settings.d2d_use = true;
	settings.d3d_use = false;
	settings.d2d_use_aa = true;
	settings.max_frame_time = 1.0f / 60.0f;
}

void PathFindingGame::initialize(GameSettings& settings)
{
	settings.m_WindowWidth = 1280;
	settings.m_WindowHeight = 720;
	settings.m_WindowTitle = "Sample: PathFinding";
}

void PathFindingGame::start(void)
{
}

void PathFindingGame::end(void)
{
}

void PathFindingGame::paint(graphics::D2DRenderContext& ctx)
{
}

void PathFindingGame::tick(double deltaTime)
{
}

void PathFindingGame::debug_ui()
{
}
