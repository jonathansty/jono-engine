#include "PathFindingTest.pch.h"
#include "PathFindingGame.h"
#include "InputManager.h"
#include "Graphics/ShaderCache.h"

#include <queue>

REGISTER_TYPE("/Types/Games/PathFindingGame", PathFindingGame);
SERIALIZE_FN(PathFindingGame) {}

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

void PathFindingGame::ConfigureEngine(EngineCfg& settings)
{
    settings.m_UseD2D = true;
    settings.m_UseD2DAA = true;
    settings.m_MSAA = MSAAMode::MSAA_4x; 
    settings.m_UseD3D = false;
}

void PathFindingGame::ConfigureGame(GameCfg& settings)
{
    settings.m_WindowWidth = 1280;
    settings.m_WindowHeight = 720;
    // settings.m_WindowTitle = "Sample: PathFinding";
}

void PathFindingGame::OnStartup(void)
{
    // Setup the navigation grid
    m_Grid._cell_size = 50.0f;
    m_Grid._width = 10;
    m_Grid._height = 10;
    m_Grid._cells.resize(m_Grid._width * m_Grid._height);
    for (u32 i = 0; i < u32(m_Grid._cells.size()); ++i)
    {
        int x = i % m_Grid._width;
        int y = i / m_Grid._height;

        m_Grid._cells[i].idx = i;
        m_Grid._cells[i].centre = (m_Grid._cell_size / 2.0f) + float2{ x * m_Grid._cell_size, y * m_Grid._cell_size };
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

    m_NavGrid = construct_grid_from_pos(m_Grid, m_Start.x, m_Start.y);

    m_Start = float2(0.0);
    m_End = float2(9, 9);

    m_ViewTranslation = float3(0.0, 0.0, 0.0);

    m_Bitmap = Bitmap::load("res:/Bitmaps/Player/player_tilesheet.png");
}

void PathFindingGame::OnShutdown(void)
{
}

void PathFindingGame::OnPaint2D(Graphics::D2DRenderContext& ctx)
{
    JONO_EVENT();

    float4x4 zoom = float4x4::scale(m_Zoom);
    auto engine = GameEngine::instance();
    float4x4 view = hlslpp::mul(float4x4::translation(-m_ViewTranslation), zoom);
    ctx.set_view_matrix(view);
    ctx.draw_background(MK_COLOR(33, 33, 33, 255));

    ctx.set_color(MK_COLOR(0, 255, 0, 255));

    for (size_t i = 0; i < m_Grid._cells.size(); ++i)
    {
        NavGridCell const& cell = m_Grid._cells[i];
        NavCell const& nav_cell = m_NavGrid._cells[i];

        ctx.fill_ellipse(cell.centre, 1.0, 1.0);
        if (cell.passable)
        {
            ctx.draw_rect(cell.centre - float2(m_Grid._cell_size * 0.5f), cell.centre + float2(m_Grid._cell_size * 0.5f));
        }
        else
        {
            ctx.fill_rect(cell.centre - float2(m_Grid._cell_size * 0.5f), cell.centre + float2(m_Grid._cell_size * 0.5f));
        }

        if (nav_cell.previous != nullptr)
        {
            // Point at the previous cell
            constexpr f32 line_length = 25.0f;

            u32 previous_index = m_NavGrid.get_cell_idx(nav_cell.previous);
            float2 direction = m_Grid._cells[previous_index].centre - cell.centre;
            direction = hlslpp::normalize(direction);
            ctx.draw_line(cell.centre, cell.centre + line_length * direction, 1.0f);
        }
    }

    ctx.set_color(MK_COLOR(255, 255, 0, 125));

    NavCell* result = m_NavGrid.find_path(m_End.x, m_End.y);
    while (result)
    {
        u32 idx = m_NavGrid.get_cell_idx(result);
        NavGridCell const& cell = m_Grid._cells[idx];

        // Draw line
        if (result->previous)
        {
            u32 prev_idx = m_NavGrid.get_cell_idx(result->previous);
            NavGridCell const& prev_cell = m_Grid._cells[prev_idx];
            ctx.draw_line(cell.centre, prev_cell.centre, 1.0f);
        }

        result = result->previous;
    }

    ctx.set_color(MK_COLOR(255, 0, 0, 255));
    if (NavGridCell* cell = get_cell(m_Grid, m_Start.x, m_Start.y); cell)
    {
        ctx.fill_ellipse(cell->centre, 1.0, 1.0);
        ctx.draw_ellipse(cell->centre, m_Grid._cell_size * 0.5f, m_Grid._cell_size * 0.5f);
    }

    ctx.set_color(MK_COLOR(0, 0, 255, 125));

    if (NavGridCell* cell = get_cell(m_Grid, m_Start.x, m_Start.y); cell)
    {
        ctx.fill_ellipse(cell->centre, 1.0, 1.0);
        ctx.draw_ellipse(cell->centre, m_Grid._cell_size * 0.5f, m_Grid._cell_size * 0.5f);
    }

    constexpr u32 s_frame_height = 61;
    constexpr f32 s_frame_width = 50.667;
    D2D1_RECT_F re;
    re.top = 0;
    re.bottom = s_frame_height;

    re.left = (s_counter)*s_frame_width;
    re.right = (s_counter + 1) * s_frame_width;
    ctx.draw_bitmap(m_Bitmap.get(), { 0, 0 }, re);

    ctx.set_color(MK_COLOR(255, 0, 0, 255));
    ctx.draw_ellipse(0, 0, 50, 50);
    ctx.fill_ellipse(0, -60, 45, 45);
}

void rebuild_shaders()
{
    using namespace Graphics;

    ShaderCache::instance()->reload_all();
}

void PathFindingGame::OnUpdate(double deltaTime)
{
    JONO_EVENT();

    if (!GameEngine::instance()->is_input_captured())
    {
        auto& input = GameEngine::instance()->get_input();

        float2 pos = GameEngine::instance()->get_mouse_pos_in_viewport();
        float4x4 view = hlslpp::mul(float4x4::translation(-m_ViewTranslation), float4x4::scale(m_Zoom));

        pos = hlslpp::mul(hlslpp::inverse(view), float4(pos, 0.4f, 1.0f)).xy;

        float2 world_pos = m_ViewTranslation.xy + pos;
        float2 grid_pos = world_pos / m_Grid._cell_size;
        grid_pos = hlslpp::clamp(grid_pos, float2(0, 0), float2(m_Grid._width, m_Grid._height));

        if (input->is_key_down(KeyCode::LShift) && input->is_mouse_button_pressed(SDL_BUTTON_LEFT))
        {
            m_Start = uint2(u32(grid_pos.x), u32(grid_pos.y));
            m_NavGrid = construct_grid_from_pos(m_Grid, m_Start.x, m_Start.y);
        }
        else if (input->is_key_down(KeyCode::LControl) && input->is_mouse_button_pressed(SDL_BUTTON_LEFT))
        {
            m_End = uint2(u32(grid_pos.x), u32(grid_pos.y));
        }
        else if (input->is_key_down(KeyCode::LAlt) && input->is_mouse_button_pressed(SDL_BUTTON_LEFT))
        {
            if (auto cell = get_cell(m_Grid, u32(grid_pos.x), u32(grid_pos.y)); cell)
            {
                cell->passable = !cell->passable;
            }
            m_NavGrid = construct_grid_from_pos(m_Grid, m_Start.x, m_Start.y);
        }
        else if (input->is_mouse_button_down(SDL_BUTTON_LEFT))
        {
            int2 delta = input->get_mouse_delta();
            m_ViewTranslation -= float3(delta, 0.0f);
        }

        if (f32 scroll = input->get_scroll_delta(); scroll != 0.0f)
        {
            m_Zoom += scroll * 0.05f;
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

        if (input->is_key_pressed(KeyCode::R) && input->is_key_down(KeyCode::LControl))
        {
            rebuild_shaders();
        }
    }

    s_time -= (f32)deltaTime;
    if (s_time < 0.0f)
    {
        s_counter = (s_counter + 1) % 2;
        s_time = 0.25f;
    }
}

void PathFindingGame::OnDebugUI()
{
    ImGui::Begin("PathFindingGame");

    u32 tmp[2] = { m_Start.x, m_Start.y };
    if (ImGui::InputScalarN("Start", ImGuiDataType_U32, &tmp, 2))
    {
        m_Start = { tmp[0], tmp[1] };
    }

    tmp[0] = m_End.x;
    tmp[1] = m_End.y;
    if (ImGui::InputScalarN("End", ImGuiDataType_U32, &tmp, 2))
    {
        m_End = { tmp[0], tmp[1] };
    }

    ImGui::End();
}
