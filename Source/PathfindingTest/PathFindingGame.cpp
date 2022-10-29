#include "PathFindingTest.pch.h"
#include "PathFindingGame.h"
#include "InputManager.h"
#include "Graphics/ShaderCache.h"

#include <queue>


static u32 s_counter = 0;
static f32 s_time = 0.0f;

NavPathGrid construct_grid_from_pos(NavGrid& grid, u32 x0, u32 y0)
{
	JONO_EVENT();
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

void PathFindingGame::configure_engine(EngineCfg& settings)
{
	settings.d2d_use = true;
	settings.d3d_use = false;
	settings.d2d_use_aa = true;
	settings.d3d_msaa_mode = MSAAMode::MSAA_4x;
	settings.max_frame_time = 1.0f / 60.0f;
}

void PathFindingGame::initialize(GameCfg& settings)
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
	for (u32 i = 0; i < u32(_grid._cells.size()); ++i)
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

	_view_translation = float3(0.0, 0.0,0.0);

	_bitmap = Bitmap::load("Resources/Tests/Bitmaps/character.png");
}

void PathFindingGame::end(void)
{
}

void PathFindingGame::paint(Graphics::D2DRenderContext& ctx)
{
	JONO_EVENT();
	
	float4x4 zoom = float4x4::scale(_zoom);
	auto engine = GameEngine::instance();
	float4x4 view = hlslpp::mul(float4x4::translation(-_view_translation), zoom);
	ctx.set_view_matrix(view);
	ctx.draw_background(MK_COLOR(33, 33, 33, 255));

	ctx.set_color(MK_COLOR(0, 255, 0, 255));

	for (size_t i = 0; i < _grid._cells.size(); ++i)
	{
		NavGridCell const& cell = _grid._cells[i];
		NavCell const& nav_cell = _nav_grid._cells[i];

		ctx.fill_ellipse(cell.centre, 1.0, 1.0);
		if (cell.passable)
		{
			ctx.draw_rect(cell.centre - float2(_grid._cell_size * 0.5f), cell.centre + float2(_grid._cell_size * 0.5f));
		}
		else
		{
			ctx.fill_rect(cell.centre - float2(_grid._cell_size * 0.5f), cell.centre + float2(_grid._cell_size * 0.5f));
		}

		if(nav_cell.previous != nullptr)
		{
			// Point at the previous cell
			constexpr f32 line_length = 25.0f;

			u32 previous_index = _nav_grid.get_cell_idx(nav_cell.previous);
			float2 direction = _grid._cells[previous_index].centre - cell.centre;
			direction = hlslpp::normalize(direction);
			ctx.draw_line(cell.centre, cell.centre + line_length * direction , 1.0f);
		}
	}

	ctx.set_color(MK_COLOR(255, 255, 0, 125));

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
			ctx.draw_line(cell.centre, prev_cell.centre, 1.0f);
		}

		result = result->previous;
	}


	ctx.set_color(MK_COLOR(255, 0, 0, 255));
	if (NavGridCell* cell = get_cell(_grid, _start.x, _start.y); cell)
	{
		ctx.fill_ellipse(cell->centre, 1.0, 1.0);
		ctx.draw_ellipse(cell->centre, _grid._cell_size * 0.5f, _grid._cell_size * 0.5f);
	}

	ctx.set_color(MK_COLOR(0, 0, 255, 125));

	if (NavGridCell* cell = get_cell(_grid, _start.x, _start.y); cell)
	{
		ctx.fill_ellipse(cell->centre, 1.0, 1.0);
		ctx.draw_ellipse(cell->centre, _grid._cell_size * 0.5f, _grid._cell_size * 0.5f);
	}


	constexpr u32 s_frame_height = 61;
	constexpr f32 s_frame_width = 50.667;
	D2D1_RECT_F re;
	re.top = 0;
	re.bottom = s_frame_height;

	re.left = (s_counter) * s_frame_width;
	re.right = (s_counter+1) * s_frame_width;
	ctx.draw_bitmap(_bitmap.get(), { 0, 0 }, re);

	ctx.set_color(MK_COLOR(255, 0, 0, 255));
	ctx.draw_ellipse(0, 0, 50, 50);
	ctx.fill_ellipse(0, -60, 45, 45);

}

void rebuild_shaders()
{
	using namespace Graphics;

	ShaderCache::instance()->reload_all();
}

void PathFindingGame::tick(double deltaTime)
{
	JONO_EVENT();

	if (!GameEngine::instance()->is_input_captured())
	{
		auto& input = GameEngine::instance()->get_input();

		float2 pos = GameEngine::instance()->get_mouse_pos_in_viewport();
		float4x4 view = hlslpp::mul(float4x4::translation(-_view_translation), float4x4::scale(_zoom));

		pos = hlslpp::mul(hlslpp::inverse(view), float4(pos, 0.4f, 1.0f)).xy;

		float2 world_pos = _view_translation.xy + pos;
		float2 grid_pos = world_pos / _grid._cell_size;
		grid_pos = hlslpp::clamp(grid_pos, float2(0, 0), float2(_grid._width, _grid._height));

		if (input->is_key_down(KeyCode::LShift) && input->is_mouse_button_pressed(0))
		{
			_start = uint2(u32(grid_pos.x), u32(grid_pos.y));
			_nav_grid = construct_grid_from_pos(_grid, _start.x, _start.y);
		}
		else if (input->is_key_down(KeyCode::LControl) && input->is_mouse_button_pressed(0))
		{
			_end = uint2(u32(grid_pos.x), u32(grid_pos.y));
		}
		else if (input->is_key_down(KeyCode::LAlt) && input->is_mouse_button_pressed(0))
		{
			if (auto cell = get_cell(_grid, u32(grid_pos.x), u32(grid_pos.y)); cell)
			{
				cell->passable = !cell->passable;
			}
			_nav_grid = construct_grid_from_pos(_grid, _start.x, _start.y);
		}
		else if (input->is_mouse_button_down(0))
		{
			int2 delta = input->get_mouse_delta();
			_view_translation -= float3(delta, 0.0f);
		}

		if (f32 scroll = input->get_scroll_delta(); scroll != 0.0f)
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

		if (input->is_key_pressed(KeyCode::R) && input->is_key_down(KeyCode::LControl))
		{
			rebuild_shaders();
		}
	}

	s_time -= (f32)deltaTime;
	if(s_time < 0.0f)
	{
		s_counter = (s_counter + 1) % 2;
		s_time = 0.25f;
	}
}

void PathFindingGame::debug_ui()
{
	ImGui::Begin("PathFindingGame");

	u32 tmp[2] = { _start.x, _start.y };
	if(ImGui::InputScalarN("Start",ImGuiDataType_U32, &tmp, 2))
	{
		_start = { tmp[0], tmp[1] };
	}

	tmp[0] = _end.x;
	tmp[1] = _end.y;
	if(ImGui::InputScalarN("End",ImGuiDataType_U32, &tmp, 2))
	{
		_end = { tmp[0], tmp[1] };
	}

	ImGui::End();
}

