#include "PathFindingTest.pch.h"
#include "PathFindingGame.h"
#include "InputManager.h"

#include <queue>




NavPathGrid construct_grid_from_pos(NavGrid& grid, u32 x0, u32 y0)
{
	NavPathGrid temp = NavPathGrid::from(grid);
	u32 width = grid.m_Width;
	u32 idx = x0 + y0 * width;

	// Add start to frontier
	std::queue<NavCell*> frontier;
	std::set<NavCell*> reached;
	frontier.push(&temp._cells[idx]);
	reached.insert(&temp._cells[idx]);

	while (!frontier.empty())
	{
		// Get the first in the frontier queue
		NavCell* current = frontier.front();
		frontier.pop();

		// Retrieve all neighbours and, if neighbour not in reached consider this next iteration in frontier
		std::vector<NavCell*> const& neighbours = current->neighbours;
		for (NavCell* n : neighbours)
		{
			if (n->passable && reached.find(n) == reached.end())
			{
				n->previous = current;

				frontier.push(n);
				reached.insert(n);
			}
		}
	}

	return temp;
}

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

		m_Grid.m_GridCells[i].idx = i;
		m_Grid.m_GridCells[i].centre = float2{ x*m_Grid.m_CellSize, y*m_Grid.m_CellSize };
	}

	// Setup the grid
	get_cell(m_Grid, 0, 3)->passable = false;
	get_cell(m_Grid, 1, 3)->passable = false;
	get_cell(m_Grid, 2, 3)->passable = false;
	get_cell(m_Grid, 3, 3)->passable = false;
	get_cell(m_Grid, 4, 3)->passable = false;
	get_cell(m_Grid, 5, 3)->passable = false;
	get_cell(m_Grid, 6, 3)->passable = false;

	get_cell(m_Grid, 3, 6)->passable = false;
	get_cell(m_Grid, 4, 6)->passable = false;
	get_cell(m_Grid, 5, 6)->passable = false;
	get_cell(m_Grid, 6, 6)->passable = false;
	get_cell(m_Grid, 8, 6)->passable = false;
	get_cell(m_Grid, 7, 6)->passable = false;
	get_cell(m_Grid, 9, 6)->passable = false;

	m_NavigationGrid = construct_grid_from_pos(m_Grid, m_Start.x, m_Start.y);

	m_Start = float2(0.0);
	m_End = float2(9, 9);

	m_ViewTranslation = float2(0.0, 0.0);
}

void PathFindingGame::end(void)
{
}

void PathFindingGame::paint(Graphics::D2DRenderContext& ctx)
{
	
	float3x3 zoom = float3x3::scale(m_ViewZoom);
	auto engine = GameEngine::instance();
	float3x3 view = hlslpp::mul(float3x3::translation(-m_ViewTranslation), zoom);
	ctx.set_view_matrix(view);
	ctx.draw_background(MK_COLOR(33, 33, 33, 255));

	ctx.set_color(MK_COLOR(0, 255, 0, 255));
	// Paint the navigation grid
	for (size_t i = 0; i < m_Grid.m_GridCells.size(); ++i)
	{
		NavGridCell const& cell = m_Grid.m_GridCells[i];
		NavCell const& nav_cell = m_NavigationGrid._cells[i];

		ctx.fill_ellipse(cell.centre, 1.0, 1.0);
		if (cell.passable)
		{
			//ctx.draw_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
			ctx.draw_rect(cell.centre - float2(m_Grid.m_CellSize * 0.5f), cell.centre + float2(m_Grid.m_CellSize * 0.5f));
		}
		else
		{
			//ctx.fill_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
			ctx.fill_rect(cell.centre - float2(m_Grid.m_CellSize * 0.5f), cell.centre + float2(m_Grid.m_CellSize * 0.5f));
		}

		if(nav_cell.previous != nullptr)
		{
			// Point at the previous cell
			constexpr f32 line_length = 25.0f;

			u32 previous_index = m_NavigationGrid.get_cell_idx(nav_cell.previous);
			float2 direction = m_Grid.m_GridCells[previous_index].centre - cell.centre;
			direction = hlslpp::normalize(direction);
			ctx.draw_line(cell.centre, cell.centre + line_length * direction , 0.2f);
		}
	}

	ctx.set_color(MK_COLOR(255, 255, 0, 255));

	NavCell* result = m_NavigationGrid.find_path(m_End.x, m_End.y);
	while(result)
	{
		u32 idx = m_NavigationGrid.get_cell_idx(result);
		NavGridCell const& cell = m_Grid.m_GridCells[idx];

		// Draw line
		if(result->previous)
		{
			u32 prev_idx = m_NavigationGrid.get_cell_idx(result->previous);
			NavGridCell const& prev_cell = m_Grid.m_GridCells[prev_idx];
			ctx.draw_line(cell.centre, prev_cell.centre, 0.5f);
		}

		result = result->previous;
	}


	ctx.set_color(MK_COLOR(255, 0, 0, 255));
	if (NavGridCell* cell = get_cell(m_Grid, m_Start.x, m_Start.y); cell)
	{
		ctx.fill_ellipse(cell->centre, 1.0, 1.0);
		ctx.draw_ellipse(cell->centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
	}

	ctx.set_color(MK_COLOR(0, 0, 255, 255));

	if (NavGridCell* cell = get_cell(m_Grid, m_Start.x, m_Start.y); cell)
	{
		ctx.fill_ellipse(cell->centre, 1.0, 1.0);
		ctx.draw_ellipse(cell->centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
	}
}

void PathFindingGame::tick(double deltaTime)
{
	auto& input = GameEngine::instance()->get_input();
	if(input->is_mouse_button_down(0))
	{
		int2 delta = input->get_mouse_delta();
		m_ViewTranslation -= delta;
	}

	if(f32 scroll = input->get_scroll_delta(); scroll != 0.0f)
	{
		m_ViewZoom += scroll * 0.05f;
	}

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

