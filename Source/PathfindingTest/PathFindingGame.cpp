#include "PathFindingTest.pch.h"
#include "PathFindingGame.h"
#include "InputManager.h"

#include <queue>




NavPathGrid construct_grid_from_pos(NavGrid& grid, u32 x0, u32 y0)
{
	NavPathGrid temp = NavPathGrid::from(grid);
	u32 width = grid._width;
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
	_grid._cell_size = 50.0f;
	_grid._width = 10;
	_grid._height = 10;
	_grid._cells.resize(_grid._width * _grid._height);
	for (size_t i = 0; i < _grid._cells.size(); ++i)
	{
		int x = i % _grid._width;
		int y = i / _grid._height; 

		_grid._cells[i].idx = i;
		_grid._cells[i].centre = (_grid._cell_size / 2.0f) + float2{ x * _grid._cell_size, y * _grid._cell_size };
	}

	// Setup the grid
	get_cell(_grid, 0, 3)->passable = false;
	get_cell(_grid, 1, 3)->passable = false;
	get_cell(_grid, 2, 3)->passable = false;
	get_cell(_grid, 3, 3)->passable = false;
	get_cell(_grid, 4, 3)->passable = false;
	get_cell(_grid, 5, 3)->passable = false;
	get_cell(_grid, 6, 3)->passable = false;

	get_cell(_grid, 3, 6)->passable = false;
	get_cell(_grid, 4, 6)->passable = false;
	get_cell(_grid, 5, 6)->passable = false;
	get_cell(_grid, 6, 6)->passable = false;
	get_cell(_grid, 8, 6)->passable = false;
	get_cell(_grid, 7, 6)->passable = false;
	get_cell(_grid, 9, 6)->passable = false;

	_nav_grid = construct_grid_from_pos(_grid, _start.x, _start.y);

	_start = float2(0.0);
	_end = float2(9, 9);

	_view_translation = float2(0.0, 0.0);
}

void PathFindingGame::end(void)
{
}

void PathFindingGame::paint(Graphics::D2DRenderContext& ctx)
{
	
	float3x3 zoom = float3x3::scale(_zoom);
	auto engine = GameEngine::instance();
	float3x3 view = hlslpp::mul(float3x3::translation(-_view_translation), zoom);
	ctx.set_view_matrix(view);
	ctx.draw_background(MK_COLOR(33, 33, 33, 255));

	ctx.set_color(MK_COLOR(0, 255, 0, 255));
	// Paint the navigation grid
	for (size_t i = 0; i < _grid._cells.size(); ++i)
	{
		NavGridCell const& cell = _grid._cells[i];
		NavCell const& nav_cell = _nav_grid._cells[i];

		ctx.fill_ellipse(cell.centre, 1.0, 1.0);
		if (cell.passable)
		{
			//ctx.draw_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
			ctx.draw_rect(cell.centre - float2(_grid._cell_size * 0.5f), cell.centre + float2(_grid._cell_size * 0.5f));
		}
		else
		{
			//ctx.fill_ellipse(cell.centre, m_Grid.m_CellSize * 0.5f, m_Grid.m_CellSize * 0.5f);
			ctx.fill_rect(cell.centre - float2(_grid._cell_size * 0.5f), cell.centre + float2(_grid._cell_size * 0.5f));
		}

		if(nav_cell.previous != nullptr)
		{
			// Point at the previous cell
			constexpr f32 line_length = 25.0f;

			u32 previous_index = _nav_grid.get_cell_idx(nav_cell.previous);
			float2 direction = _grid._cells[previous_index].centre - cell.centre;
			direction = hlslpp::normalize(direction);
			ctx.draw_line(cell.centre, cell.centre + line_length * direction , 0.2f);
		}
	}

	ctx.set_color(MK_COLOR(255, 255, 0, 255));

	NavCell* result = _nav_grid.find_path(_end.x, _end.y);
	while(result)
	{
		u32 idx = _nav_grid.get_cell_idx(result);
		NavGridCell const& cell = _grid._cells[idx];

		// Draw line
		if(result->previous)
		{
			u32 prev_idx = _nav_grid.get_cell_idx(result->previous);
			NavGridCell const& prev_cell = _grid._cells[prev_idx];
			ctx.draw_line(cell.centre, prev_cell.centre, 0.5f);
		}

		result = result->previous;
	}


	ctx.set_color(MK_COLOR(255, 0, 0, 255));
	if (NavGridCell* cell = get_cell(_grid, _start.x, _start.y); cell)
	{
		ctx.fill_ellipse(cell->centre, 1.0, 1.0);
		ctx.draw_ellipse(cell->centre, _grid._cell_size * 0.5f, _grid._cell_size * 0.5f);
	}

	ctx.set_color(MK_COLOR(0, 0, 255, 255));

	if (NavGridCell* cell = get_cell(_grid, _start.x, _start.y); cell)
	{
		ctx.fill_ellipse(cell->centre, 1.0, 1.0);
		ctx.draw_ellipse(cell->centre, _grid._cell_size * 0.5f, _grid._cell_size * 0.5f);
	}
}

void PathFindingGame::tick(double deltaTime)
{
	auto& input = GameEngine::instance()->get_input();

	float2 pos = GameEngine::instance()->get_mouse_pos_in_viewport();
	float2 world_pos = _view_translation + pos;
	float2 grid_pos = world_pos / _grid._cell_size;
	grid_pos = hlslpp::clamp(grid_pos, float2(0, 0), float2(_grid._width, _grid._height));

	if (input->is_key_down(KeyCode::Shift) && input->is_mouse_button_pressed(0))
	{
		_start = uint2(u32(grid_pos.x), u32(grid_pos.y));
		_nav_grid = construct_grid_from_pos(_grid, _start.x, _start.y);
	}
	else if (input->is_key_down(KeyCode::Control) && input->is_mouse_button_pressed(0))
	{
		_end = uint2(u32(grid_pos.x), u32(grid_pos.y));
	}
	else if (input->is_key_down(KeyCode::LeftAlt) && input->is_mouse_button_pressed(0))
	{
		if (auto cell = get_cell(_grid, grid_pos.x, grid_pos.y); cell)
		{
			cell->passable = !cell->passable;
		}
		_nav_grid = construct_grid_from_pos(_grid, _start.x, _start.y);
	}
	else if(input->is_mouse_button_down(0))
	{
		int2 delta = input->get_mouse_delta();
		_view_translation -= delta;
	}

	if(f32 scroll = input->get_scroll_delta(); scroll != 0.0f)
	{
		_zoom += scroll * 0.05f;
	}

	if (input->is_key_down(KeyCode::Right))
	{
		_view_translation.x += 10.0f;
	}
	if (input->is_key_down(KeyCode::Left))
	{
		_view_translation.x -= 10.0f;
	}
	if (input->is_key_down(KeyCode::Up))
	{
		_view_translation.y -= 10.0f;
	}
	if (input->is_key_down(KeyCode::Down))
	{
		_view_translation.y += 10.0f;
	}
}

void PathFindingGame::debug_ui()
{
	ImGui::Begin("PathFindingGame");

	int tmp[2] = { _start.x, _start.y };
	if(ImGui::InputInt2("Start", tmp))
	{
		_start = { tmp[0], tmp[1] };
	}

	tmp[0] = _end.x;
	tmp[1] = _end.y;
	if (ImGui::InputInt2("End", tmp))
	{
		_end = { tmp[0], tmp[1] };
	}

	ImGui::End();
}

