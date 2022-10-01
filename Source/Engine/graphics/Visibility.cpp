#include "engine.pch.h"
#include "Visibility.h"
#include "RenderWorld.h"
#include "RendererDebug.h"

void VisibilityManager::reset()
{
	_all_instances.clear();
	for(u32 f = 0; f < VisibilityFrustum_Count; ++f )
	{
		_visible_instances[f].clear();
	}
}

void VisibilityManager::add_instance(RenderWorldInstance* inst)
{
	_all_instances.push_back(inst);
}

void VisibilityManager::run(VisibilityParams const& params)
{
	JONO_EVENT();

	for(u32 f = 0; f < VisibilityFrustum_Count; ++f )
	{
		_visible_instances[f].reserve(_all_instances.size());

		for (RenderWorldInstance* inst : _all_instances)
		{
			bool is_visible = true;

			// Do visibility check
			Math::AABB box = inst->_model->get()->get_bounding_box();

			// #TODO: Technically this is *correct* but for some reason instances still get culled to early on the left of the frustum
			float3 radius = box.size() / 2.0f;

			// For now just do position checking of the instance
			float3 inst_position = inst->_transform._41_42_43;
			float3 bounding_box_center = inst->_model->get()->get_bounding_box().center();
			inst_position += bounding_box_center;

			radius = hlslpp::mul(inst->_transform, float4(radius, 0.0f)).xyz;

			// Do visibility test against the frustum using a sphere.
			is_visible = Math::test_frustum_sphere(params.frustum[f], inst_position, radius.x);

			if (Graphics::RendererDebugTool::s_force_all_visible)
			{
				is_visible = true;
			}

			if (is_visible)
			{
				_visible_instances[f].push_back(inst);
			}
		}
	}

}
