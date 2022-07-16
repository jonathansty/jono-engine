#include "PathFindingTest.pch.h"
#include "PathFindingGame.h"
#include "InputManager.h"

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
	// Setup the navigation grid
	m_Grid.m_CellSize = 50.0f;
	m_Grid.m_Width = 10;
	m_Grid.m_Height = 10;
	m_Grid.m_GridCells.resize(m_Grid.m_Width * m_Grid.m_Height);
	for (size_t i = 0; i < m_Grid.m_GridCells.size(); ++i)
	{
		int x = i % m_Grid.m_Width;
		int y = i / m_Grid.m_Height; 

		m_Grid.m_GridCells[i].centre = float2{ x*m_Grid.m_CellSize, y*m_Grid.m_CellSize };
	}

	m_Start = &get_cell(m_Grid, 0, 0);
	m_End = &get_cell(m_Grid, 5, 8);

	get_cell(m_Grid, 0, 3).passable = false;
	get_cell(m_Grid, 1, 3).passable = false;
	get_cell(m_Grid, 2, 3).passable = false;
	get_cell(m_Grid, 3, 3).passable = false;
	get_cell(m_Grid, 4, 3).passable = false;
	get_cell(m_Grid, 5, 3).passable = false;
	get_cell(m_Grid, 6, 3).passable = false;

	get_cell(m_Grid, 3, 6).passable = false;
	get_cell(m_Grid, 4, 6).passable = false;
	get_cell(m_Grid, 5, 6).passable = false;
	get_cell(m_Grid, 6, 6).passable = false;
	get_cell(m_Grid, 8, 6).passable = false;
	get_cell(m_Grid, 7, 6).passable = false;
	get_cell(m_Grid, 9, 6).passable = false;

	m_ViewTranslation = float2(0.0, 0.0);
}

void PathFindingGame::end(void)
{
}

void PathFindingGame::paint(Graphics::D2DRenderContext& ctx)
{
	
	float3x3 view = float3x3::translation(-m_ViewTranslation);
	ctx.set_view_matrix(view);
	ctx.draw_background(MK_COLOR(33, 33, 33, 255));

	ctx.set_color(MK_COLOR(0, 255, 0, 255));
	// Paint the navigation grid

	for (size_t i = 0; i < m_Grid.m_GridCells.size(); ++i)
	{
		NavGridCell const& cell = m_Grid.m_GridCells[i];
		ctx.fill_ellipse(cell.centre, 1.0, 1.0);
		if (cell.passable)
		{
			ctx.draw_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
		}
		else
		{
			ctx.fill_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
		}
	}


	ctx.set_color(MK_COLOR(255, 0, 0, 255));
	if (m_Start)
	{
		NavGridCell const& cell = *m_Start;
		ctx.fill_ellipse(cell.centre, 1.0, 1.0);
		ctx.draw_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
	}

	ctx.set_color(MK_COLOR(0, 0, 255, 255));
	if (m_End)
	{
		NavGridCell const& cell = *m_End;
		ctx.fill_ellipse(cell.centre, 1.0, 1.0);
		ctx.draw_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
	}
}

void PathFindingGame::tick(double deltaTime)
{
	auto& input = GameEngine::instance()->get_input();
	if (input->is_key_down(KeyCode::Right))
	{
		m_ViewTranslation.x += 10.0f;
	}
	if (input->is_key_down(KeyCode::Left))
	{
		m_ViewTranslation.x -= 10.0f;
	}
	if (input->is_key_down(KeyCode::Up))
	{
		m_ViewTranslation.y -= 10.0f;
	}
	if (input->is_key_down(KeyCode::Down))
	{
		m_ViewTranslation.y += 10.0f;
	}
}

void PathFindingGame::debug_ui()
{
}


struct NavPathGrid
{
	struct Cell
	{
		// index to previously visited cell
		u32 previous = -1;
	};

	std::vector<Cell> _cells;

	Cell* get_cell(u32 idx)
	{
		return &_cells[idx];
	}

	static NavPathGrid from(NavGrid const& grid)
	{
		NavPathGrid result{};
		result._cells.resize(grid.m_GridCells.size());

		return result;
	}

};

NavPathCell find_path(NavGrid& grid, u32 x0, u32 y0, u32 x1, u32 y1)
{
	NavPathGrid temp = NavPathGrid::from(grid);
	u32 width = grid.m_Width;

	u32 idx = x0 + y0 * width;

	// Add start to frontier
	std::vector<NavPathGrid::Cell*> frontier;
	std::vector<NavPathGrid::Cell*> reached;
	frontier.push_back(&temp._cells[idx]);

	reached.push_back(&temp._cells[idx]);






}
