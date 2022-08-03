#pragma once

#include "Engine/AbstractGame.h"

struct NavGridCell
{
	u32 idx = u32(-1);

	float2 centre = float2(0,0);
	bool passable = true;
};

struct NavGrid
{
	// Dimensions of the grid
	u32 _width;
	u32 _height;

	f32 _cell_size;

	std::vector<NavGridCell> _cells;
};

inline NavGridCell* get_cell(NavGrid& grid, u32 x, u32 y)
{
	if (x >= 0 && x < grid._width && y >= 0 && y < grid._height)
	{
		return &grid._cells[x + y * grid._width];
	}
	return nullptr;
}

struct NavCell
{
	bool reached = false;

	bool passable = true;

	// index to previously visited cell
	NavCell* previous = nullptr;

	// Reference neighbouring cell indices
	std::vector<NavCell*> neighbours;
};

// Intermediate grid state. Each cell idx maps to an index in the original grid
struct NavPathGrid
{
	// Cell storage
	std::vector<NavCell> _cells;

	NavCell* get_cell(u32 idx)
	{
		if (idx >= 0 && idx < _cells.size())
			return &_cells[idx];

		return nullptr;
	}

	NavCell* get_cell(u32 x, u32 y)
	{
		if(x >= 0 && x < _width && y >= 0 && y < _height)
		{
			return &_cells[x + y * _width];
		}
		return nullptr;
	}

	u32 get_cell_idx(NavCell* c) const
	{
		return u32(c - _cells.data());
	}

	NavCell* find_path(u32 x, u32 y)
	{
		NavCell* c = get_cell(x + y * _width);
		if (c && c->previous != nullptr)
		{
			return c;
		}

		return nullptr;
	}

	void setup_links(NavGrid const& grid, bool diagonals = false)
	{
		// Hookup neighbours
		for (u32 i = 0; i < _cells.size(); ++i)
		{
			u32 x = i % _width;
			u32 y = i / _width;

			NavCell& current = _cells[i];

			// Copy from grid to nav cell
			current.passable = grid._cells[i].passable;

			// Horizontal
			if (NavCell* c = get_cell(x- 1, y)) current.neighbours.push_back(c);
			if (NavCell* c = get_cell(x+1,y))   current.neighbours.push_back(c);

			// Vertical
			if (NavCell* c = get_cell(x,y+1)) current.neighbours.push_back(c);
			if (NavCell* c = get_cell(x, y-1)) current.neighbours.push_back(c);

			// Diagonal
			if(diagonals)
			{
				if (NavCell* c = get_cell(x- 1, y - 1)) current.neighbours.push_back(c);
				if (NavCell* c = get_cell(x- 1, y + 1)) current.neighbours.push_back(c);

				if (NavCell* c = get_cell(x + 1,y - 1 ))   current.neighbours.push_back(c);
				if (NavCell* c = get_cell(x + 1,y + 1 ))   current.neighbours.push_back(c);
			}
		}
	}

	static NavPathGrid from(NavGrid const& grid)
	{
		NavPathGrid result{};
		u32 size = u32(grid._cells.size());

		result._width = grid._width;
		result._height = grid._height;
		result._cells.resize(size);

		result.setup_links(grid);

		return result;
	}

	u32 _width;
	u32 _height;
};


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
	float3 _view_translation;
	f32 _zoom = 1.0f;

	uint2 _start;
	uint2 _end;

	NavGrid _grid;
	NavPathGrid _nav_grid;
};