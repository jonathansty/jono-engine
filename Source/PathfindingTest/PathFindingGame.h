#pragma once

#include "Engine/AbstractGame.h"

struct NavGridCell
{
	float2 centre = float2(0,0);

	bool passable = true;
};

struct NavGrid
{
	// Dimensions of the grid
	u32 m_Width;
	u32 m_Height;

	u32 m_CellSize;

	std::vector<NavGridCell> m_GridCells;
};

struct NavPathCell
{
	NavPathCell* previous;
};

NavPathCell find_path(NavGrid& grid, u32 x0, u32 y0, u32 x1, u32 y1)
{



}

inline NavGridCell& get_cell(NavGrid& grid, u32 x, u32 y)
{
	return grid.m_GridCells[x + y * grid.m_Width];
}

class PathFindingGame : public AbstractGame
{

public:
	void configure_engine(EngineSettings&) override;

	void initialize(GameSettings& gameSettings) override;

	void start(void) override;

	void end(void) override;

	void paint(Graphics::D2DRenderContext& ctx) override;

	void tick(double deltaTime) override;

	void debug_ui() override;

private:
	float2 m_ViewTranslation;

	NavGridCell* m_Start;
	NavGridCell* m_End;

	NavGrid m_Grid;

};