#include "engine.pch.h"

#include "GameEngine.h"

#include "Graphics/ShaderCache.h"
#include "Debug.h"
#include "RendererDebug.h"

namespace Graphics
{

bool RendererDebugTool::s_force_all_visible = false;

RendererDebugTool::RendererDebugTool(Renderer* owner)
		: DebugOverlay(true, "RendererDebug")
		, _renderer(owner)
		, _show_shadow_debug(false)
{
}

void RendererDebugTool::RenderOverlay()
{
	render_debug_tool();
	render_shader_tool();
}

void RendererDebugTool::Render3D(ID3D11DeviceContext* ctx)
{
	if (!_batch)
	{
		_batch = std::make_shared<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(ctx);
	}

	_batch->Begin();

	#if 0 
	// Debug cube rendering test
	{
        auto box = DirectX::BoundingBox({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
        Debug::Draw(_batch.get(), box, { 1.0f, 1.0f, 1.0f, 1.0f });

        box = DirectX::BoundingBox({ 10.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
        Debug::Draw(_batch.get(), box, { 1.0f, 1.0f, 1.0f, 1.0f });

        box = DirectX::BoundingBox({ 20.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
        Debug::Draw(_batch.get(), box, { 1.0f, 1.0f, 1.0f, 1.0f });
    }
	#endif



	Debug::DrawGrid(_batch.get(), float4(100.0, 0.0, 0.0, 0.0), float4(0.0, 0.0, 100.0, 0.0), float4(0.0, 0.0, 0.0, 0.0), 10, 10, float4(1.0f, 1.0f, 1.0f, 0.5f));

	// Draw axis
	Debug::DrawRay(_batch.get(), float4(0.0f), float4(1.0f, 0.0f, 0.0f, 0.0f), true, float4(1.0f, 0.0f, 0.0f, 1.0f));
	Debug::DrawRay(_batch.get(), float4(0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), true, float4(0.0f, 1.0f, 0.0f, 1.0f));
	Debug::DrawRay(_batch.get(), float4(0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), true, float4(0.0f, 0.0f, 1.0f, 1.0f));

	RenderWorld const& world = GetGlobalContext()->m_GraphicsThread->m_FrameData.m_RenderWorld;

	// Visualize camera frustums
	{
		std::vector<float4> colors = {
			float4(1.0f, 0.0f, 0.0f, 1.0f),
			float4(0.0f, 1.0f, 0.0f, 1.0f),
			float4(0.0f, 0.0f, 1.0f, 1.0f),
			float4(1.0f, 0.0f, 1.0f, 1.0f),
			float4(1.0f, 1.0f, 0.0f, 1.0f),
			float4(0.0f, 1.0f, 1.0f, 1.0f)
		};
		for (u32 i = 0; i < world.get_cameras().size(); ++i)
		{
			if(world.get_view_camera() != world.get_camera(i))
			{
				Math::Frustum frustum = _renderer->get_frustum_world(world, i);
				float4 color = colors[i % colors.size()];
				Debug::DrawFrustum(_batch.get(), frustum._corners, color);

				// Draw normals
				for(u32 plane = 0; plane < frustum._planes.size(); ++plane)
				{
					Debug::DrawRay(_batch.get(), frustum._planes[plane].center, float4(frustum._planes[plane].normal,1.0f), true, color);
				
				}
			}
		}
	}

	if (_show_shadow_debug)
	{
		float4 frustum_color = float4(0.7f, 0.0f, 0.0f, 1.0f);
		if (world.get_camera(0) != world.get_view_camera())
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
				Math::Frustum frustum = _renderer->get_cascade_frustum(world.get_camera(0), i, num_cascades);
				Debug::DrawFrustum(_batch.get(), frustum._corners, colors[i % colors.size()]);

				// Find the world space min/ max for each cascade
				float4 min = frustum._corners[0];
				float4 max = frustum._corners[0];
				for (u32 j = 1; j < frustum._corners.size(); ++j)
				{
					min = hlslpp::min(frustum._corners[j], min);
					max = hlslpp::max(frustum._corners[j], max);
				}


				// Visualize the bounding box in world space of our cascades
				float3 center = ((max + min) / 2.0f).xyz;
				float3 extents = ((max - min) / 2.0f).xyz;

				Shaders::float4 xm_color = colors[(i) % colors.size()];
				Shaders::float3 xm_center = float3(center.xyz);
				Shaders::float3 xm_extents = extents;
                Shaders::float3 up = Shaders::float3({ 0.0f, 1.0f, 0.0f });

				auto box = DirectX::BoundingBox(xm_center, xm_extents);
				Debug::Draw(_batch.get(), box, xm_color);


				{
                    auto camera = world.get_camera(0);
                    f32 n = camera->get_near();
                    f32 f = camera->get_far();

                    f32 z0 = n * pow(f / n, f32(i) / f32(num_cascades));
                    f32 z1 = n * pow(f / n, f32(i + 1) / f32(num_cascades));
                    hlslpp::projection p = hlslpp::projection(hlslpp::frustum(1, 1, n, f), hlslpp::zclip::zero);
					
                    float4x4 proj = hlslpp::inverse(float4x4::orthographic(p));

					center = float3(0.0f, 0.0f, 0.0f);
                    center = hlslpp::mul(float4(center, 1.0f), proj).xyz;

                    float4x4 invView = hlslpp::inverse(camera->get_view());
                    center = hlslpp::mul(float4(center, 1.0f), invView).xyz;

					DirectX::BoundingSphere sphere = DirectX::BoundingSphere(xm_center, 1.0f);
                    Debug::Draw(_batch.get(), sphere, xm_color);
                }
			}
		}

		if (RenderWorldLightRef const& light = world.get_light(0); light != nullptr)
		{
			static const float4 c_basic_cascade = float4(1.0f, 1.0f, 0.0f, 1.0f);
			static const float4 c_transformed = float4(0.0f, 1.0f, 0.0f, 1.0f);

			static u32 s_visualize_cascade = 0;
			if(s_EnableShadowRendering)
			{
				CascadeInfo const& info = light->get_cascade(s_visualize_cascade);
				Math::Frustum f = Math::Frustum::from_vp(info.vp);
				Debug::DrawFrustum(_batch.get(), f._corners, c_basic_cascade);

				float3 center = hlslpp::mul(float4(0.0, 0.0, 0.0, 1.0f), hlslpp::inverse(info.vp)).xyz;
				Debug::DrawRay(_batch.get(), float4(center.xyz, 1.0f), light->get_view_direction(), true, c_basic_cascade);

				float3 cascade_center = light->get_cascade(s_visualize_cascade).center;
				auto box = DirectX::BoundingBox(DirectX::XMFLOAT3{ cascade_center.x, cascade_center.y, cascade_center.z }, DirectX::XMFLOAT3{ 1.0, 1.0, 1.0 });
				Debug::Draw(_batch.get(), box);
			}

		}
	}

	_batch->End();
}

void RendererDebugTool::render_shader_tool()
{
	static bool s_open = false;
	if (s_open)
	{
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
}

void RendererDebugTool::render_debug_tool()
{
	if (ImGui::Begin("RendererDebug"), _isOpen)
	{
		ImGui::Checkbox("Enable Shadow Rendering", &Graphics::s_EnableShadowRendering);
		ImGui::Checkbox("CSM0", &Graphics::s_EnableCSM0);
        ImGui::Checkbox("CSM1", &Graphics::s_EnableCSM1);
        ImGui::Checkbox("CSM2", &Graphics::s_EnableCSM2);
        ImGui::Checkbox("CSM3", &Graphics::s_EnableCSM3);

		ImGui::Checkbox("Enable Shadow Debug", &_show_shadow_debug);
		ImGui::Checkbox("Force All Visible", &s_force_all_visible);

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
			if(_renderer->_active_cam >= world->get_cameras().size())
			{
				std::shared_ptr<RenderWorldCamera> cam = world->create_camera();

				// Copy the view camera
				RenderWorldCamera const& ref = *world->get_view_camera().get();
				RenderWorldCamera& out = *cam;
				out = ref;
			}
			world->set_active_camera(_renderer->_active_cam);
		}

		ImGui::Text("Active Camera: %d", _renderer->_active_cam);
		ImGui::Text("Draws: %d", _renderer->get_stats().n_draws);
		ImGui::Text("Primitives: %d", _renderer->get_stats().n_primitives);

		//ImVec2 current_size = ImGui::GetContentRegionAvail();
		//for (u32 i = 0; i < MAX_CASCADES; ++i)
		//{
		//	ImGui::Image(_renderer->_debug_shadow_map_srv[i].Get(), { 150, 150 });
		//	if (i != MAX_CASCADES - 1)
		//		ImGui::SameLine();
		//}

		//ImGui::Image(_renderer->_output_depth_srv, { 150, 150 });
	}
	ImGui::End();
}


}