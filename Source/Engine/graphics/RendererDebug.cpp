#include "engine.pch.h"

#include "GameEngine.h"

#include "Graphics/ShaderCache.h"
#include "Debug.h"
#include "RendererDebug.h"

namespace Graphics
{
RendererDebugTool::RendererDebugTool(Renderer* owner)
		: DebugOverlay(false, "RendererDebug")
		, _renderer(owner)
		, _show_shadow_debug(false)
{
}

void RendererDebugTool::render_overlay()
{
	render_debug_tool();
	render_shader_tool();
}

void RendererDebugTool::render_3d(ID3D11DeviceContext* ctx)
{
	if (!_batch)
	{
		_batch = std::make_shared<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(ctx);
	}

	_batch->Begin();

	Debug::DrawGrid(_batch.get(), float4(100.0, 0.0, 0.0, 0.0), float4(0.0, 0.0, 100.0, 0.0), float4(0.0, 0.0, 0.0, 0.0), 10, 10, float4(1.0f, 1.0f, 1.0f, 0.5f));

	if (_show_shadow_debug)
	{
		float4 frustum_color = float4(0.7f, 0.0f, 0.0f, 1.0f);
		auto world = GameEngine::instance()->get_render_world();
		if (world->get_camera(0) != world->get_view_camera())
		{
			std::vector<float4> colors = {
				float4(1.0f, 0.0f, 0.0f, 1.0f),
				float4(0.0f, 1.0f, 0.0f, 1.0f),
				float4(0.0f, 0.0f, 1.0f, 1.0f),
				float4(1.0f, 0.0f, 1.0f, 1.0f),
				float4(1.0f, 1.0f, 0.0f, 1.0f),
				float4(0.0f, 1.0f, 1.0f, 1.0f)
			};
			u32 num_cascades = MAX_CASCADES;
			for (u32 i = 0; i < num_cascades; ++i)
			{
				FrustumCorners frustum = _renderer->get_cascade_frustum(world->get_camera(0), i, num_cascades);
				Debug::DrawFrustum(_batch.get(), frustum, colors[i % colors.size()]);

				// Find the world space min/ max for each cascade
				float4 min = frustum[0];
				float4 max = frustum[0];
				for (u32 j = 1; j < frustum.size(); ++j)
				{
					min = hlslpp::min(frustum[j], min);
					max = hlslpp::max(frustum[j], max);
				}

				XMVECTOR xm_color;
				hlslpp::store(colors[(i) % colors.size()], (float*)&xm_color);

				// Visualize the bounding box in world space of our cascades
				float3 center = ((max + min) / 2.0f).xyz;
				float3 extents = ((max - min) / 2.0f).xyz;
				XMFLOAT3 xm_center;
				hlslpp::store(center, (float*)&xm_center);

				XMFLOAT3 xm_extents;
				hlslpp::store(extents, (float*)&xm_extents);
				auto box = DirectX::BoundingBox(xm_center, xm_extents);
				Debug::Draw(_batch.get(), box, xm_color);
			}
		}

		if (RenderWorldLightRef const& light = world->get_light(0); light != nullptr)
		{
			static const float4 c_basic_cascade = float4(1.0f, 1.0f, 0.0f, 1.0f);
			static const float4 c_transformed = float4(0.0f, 1.0f, 0.0f, 1.0f);

			static u32 s_visualize_cascade = 0;
			{
				CascadeInfo const& info = light->get_cascade(s_visualize_cascade);
				FrustumCorners corners;
				Math::calculate_frustum(corners, info.vp);
				Debug::DrawFrustum(_batch.get(), corners, c_basic_cascade);

				float3 center = hlslpp::mul(float4(0.0, 0.0, 0.0, 1.0f), hlslpp::inverse(info.vp)).xyz;
				Debug::DrawRay(_batch.get(), float4(center.xyz, 1.0f), light->get_view_direction(), true, c_basic_cascade);

				float3 cascade_center = light->get_cascade(s_visualize_cascade).center;
				auto box = DirectX::BoundingBox(XMFLOAT3{ cascade_center.x, cascade_center.y, cascade_center.z }, XMFLOAT3{ 1.0, 1.0, 1.0 });
				Debug::Draw(_batch.get(), box);
			}

		}
	}

	_batch->End();
}

void RendererDebugTool::render_shader_tool()
{
	static bool s_open = true;
	if (ImGui::Begin("Shaders", &s_open))
	{
		ImGui::BeginTable("##ShaderTable", 2);
		ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
		ImGui::TableSetupColumn("Path");
		ImGui::TableSetupColumn("Button");
		ImGui::TableHeadersRow();

		auto shaders = ShaderCache::instance()->_shaders;
		for (auto it : shaders)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("%s", it.first.path.c_str());

			ImGui::TableNextColumn();
			ImGui::PushID(it.first.path.c_str());
			if (ImGui::Button("Build"))
			{
				ShaderCache::instance()->reload(it.first);
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::End();
}

void RendererDebugTool::render_debug_tool()
{
	if (ImGui::Begin("RendererDebug"), _isOpen)
	{
		ImGui::Checkbox("Show Shadow Debug", &_show_shadow_debug);

		if (ImGui::Button("Toggle Debug Cam"))
		{
			if (_renderer->_active_cam == 0)
			{
				_renderer->_active_cam = 1;
			}
			else
			{
				_renderer->_active_cam = 0;
			}
			auto world = GameEngine::instance()->get_render_world();
			world->set_active_camera(_renderer->_active_cam);
		}
		ImGui::Text("Active Camera: %d", _renderer->_active_cam);

		ImVec2 current_size = ImGui::GetContentRegionAvail();
		for (u32 i = 0; i < MAX_CASCADES; ++i)
		{
			ImGui::Image(_renderer->_debug_shadow_map_srv[i].Get(), { 150, 150 });
			if (i != MAX_CASCADES - 1)
				ImGui::SameLine();
		}

		ImGui::Image(_renderer->_output_depth_srv, { 150, 150 });
	}
	ImGui::End();
}

}