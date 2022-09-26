#include "engine.pch.h"
#include "Visibility.h"
#include "RenderWorld.h"
#include "RendererDebug.h"

void VisibilityManager::reset()
{
	_all_instances.clear();
	_visible_instances.clear();
}

void VisibilityManager::add_instance(RenderWorldInstance* inst)
{
	_all_instances.push_back(inst);
}

void VisibilityManager::run(VisibilityParams const& params)
{
	_visible_instances.reserve(_all_instances.size());

	for(RenderWorldInstance* inst : _all_instances)
	{
		bool is_visible = true;

		// Do visibility check
		Math::AABB box = inst->_model->get()->get_bounding_box();

		f32 radius = box.width() / 2.0f;

		// For now just do position checking of the instance
		float3 inst_position = inst->_transform._41_42_43;

		//float3 bounding_box_center = inst->_model->get()->get_bounding_box().center();
		//float3 bounding_box_pos = inst_position + bounding_box_center;
		//float4x4 w = inst->_transform; 
		//w.vec3 = {};
		//w.f32_128_3[3] = 1.0f;

		//radius = hlslpp::mul(w, float4(radius)).x;

		// Do a frustum test 

		is_visible = Math::test_frustum_sphere(params.frustum, inst_position, 0.0f);

		if(Graphics::RendererDebugTool::s_force_all_visible)
		{
			is_visible = true;
		}


		if(is_visible)
		{
			_visible_instances.push_back(inst);
		}
	}

}
