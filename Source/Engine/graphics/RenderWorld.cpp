#include "engine.pch.h"

#include "GameEngine.h"
#include "RenderWorld.h"
#include "Core/Material.h"

 RenderWorld::RenderWorld(RenderWorld const& rhs)
{
	_instances.reserve(rhs._instances.size());
	_cameras.reserve(rhs._cameras.size());
	_lights.reserve(rhs._lights.size());

	for (shared_ptr<RenderWorldInstance> const& inst : rhs._instances)
	{
		_instances.push_back(std::make_shared<RenderWorldInstance>(*inst));
	}

	for (shared_ptr<RenderWorldCamera> const& cam : rhs._cameras)
	{
		_cameras.push_back(std::make_shared<RenderWorldCamera>(*cam));
	}

	for (shared_ptr<RenderWorldLight> const& light : rhs._lights)
	{
		_lights.push_back(std::make_shared<RenderWorldLight>(*light));
	}

	_active_camera = rhs._active_camera;
} 

RenderWorld& RenderWorld::operator=(RenderWorld const& rhs)
{
	_instances.reserve(rhs._instances.size());
	_cameras.reserve(rhs._cameras.size());
	_lights.reserve(rhs._lights.size());

	_instances.clear();
	_cameras.clear();
	_lights.clear();

	for (shared_ptr<RenderWorldInstance> const& inst : rhs._instances)
	{
		_instances.push_back(std::make_shared<RenderWorldInstance>(*inst));
	}

	for (shared_ptr<RenderWorldCamera> const& cam : rhs._cameras)
	{
		_cameras.push_back(std::make_shared<RenderWorldCamera>(*cam));
	}

	for (shared_ptr<RenderWorldLight> const& light : rhs._lights)
	{
		_lights.push_back(std::make_shared<RenderWorldLight>(*light));
	}

	_active_camera = rhs._active_camera;
	return *this;
}

void RenderWorld::Init()
{
	_instances.reserve(c_instance_reserve);
	_cameras.reserve(c_camera_reserve);
	_lights.reserve(c_light_reserve);
}

void RenderWorld::Clear()
{
    _instances.clear();
    _cameras.clear();
    _lights.clear();
}

std::shared_ptr<RenderWorldInstance> RenderWorld::create_instance(float4x4 transform, std::string const& mesh)
{
	std::lock_guard l{ _instance_cs };
	std::shared_ptr<RenderWorldInstance> inst = std::make_shared<RenderWorldInstance>(transform);

	ModelHandle::init_parameters params{};
	params.path = mesh;
	inst->_model = ResourceLoader::instance()->load<ModelHandle>(params, false, true);

	_instances.push_back(inst);

	return inst;
}

std::shared_ptr<RenderWorldCamera> RenderWorld::create_camera()
{
	std::lock_guard l{ _camera_cs };
	auto cam = std::make_shared<RenderWorldCamera>();
	_cameras.push_back(cam);
	return cam;
}

std::shared_ptr<RenderWorldLight> RenderWorld::create_light(RenderWorldLight::LightType type)
{
	std::lock_guard l{ _lights_cs };
	auto result = make_shared<RenderWorldLight>(type);
	_lights.push_back(result);
	return result;
}

void RenderWorld::remove_instance(std::shared_ptr<RenderWorldInstance> const& instance)
{
	std::lock_guard l{ _instance_cs };
	auto it = std::find(_instances.begin(), _instances.end(), instance);
	if (it != _instances.end())
	{
		_instances.erase(it);
	}
}

void RenderWorld::remove_light(std::shared_ptr<RenderWorldLight> const& light)
{
	std::lock_guard l{ _lights_cs };
	auto it = std::find(_lights.begin(), _lights.end(), light);
	if (it != _lights.end())
	{
		_lights.erase(it);
	}
}

std::shared_ptr<RenderWorldCamera> RenderWorld::get_view_camera() const
{
	if (_active_camera >= 0 && _active_camera < _cameras.size())
	{
		return _cameras[_active_camera];
	}
	return nullptr;
}

RenderWorldCamera::RenderWorldCamera()
		: _dirty(true)
		, _proj()
		, _view()
		, _world()
		, _settings()
	 
{
}

RenderWorldCamera::RenderWorldCamera(RenderWorldCamera const& rhs)
	: _dirty(true)
	, _proj(rhs._proj)
	, _view(rhs._view)
	,_world(rhs._world)
	,_settings(rhs._settings)
{
}

RenderWorldCamera::~RenderWorldCamera()
{
}

void RenderWorldCamera::set_aspect(f32 aspect)
{
	_settings.aspect = aspect;
	_dirty = true;
}

void RenderWorldCamera::update() const
{
	if (_dirty)
	{

		bool reverse_z = _settings.reverse_z;
		f32 near_clip = _settings.near_clip;
		f32 far_clip = _settings.far_clip;

		if (_settings.projection_type == Projection::Perspective)
		{
			hlslpp::projection proj(hlslpp::frustum::field_of_view_y(_settings.fov, _settings.aspect, near_clip, far_clip), reverse_z ? hlslpp::zclip::minus_one : hlslpp::zclip::zero);
			_proj = float4x4::perspective(proj);
		}
		else if (_settings.projection_type == Projection::Ortographic)
		{
			hlslpp::projection proj(hlslpp::frustum(_settings.width, _settings.height, near_clip, far_clip),reverse_z ? hlslpp::zclip::minus_one : hlslpp::zclip::zero);
			_proj = float4x4::orthographic(proj);
		}

		_vp = hlslpp::mul(_view, _proj);

		_dirty = false;
	}
}

RenderWorldLight::RenderWorldLight(LightType type)
		: _type(type)
		, _colour(1.0f, 1.0f, 1.0f)
		, _shadow_settings()
		, _cascade()
		, _range(10.0)
		, _cone_angle(hlslpp::radians((float1)30.0f))
		, _outer_cone_angle(hlslpp::radians((float1)30.0f))

{
}

RenderWorldLight::RenderWorldLight(RenderWorldLight const& light)
	:  RenderWorldCamera(light)
	, _type(light._type)
	, _colour(light._colour)
	, _shadow_settings(light._shadow_settings)
	, _range(light._range)
	, _cone_angle(light._cone_angle)
	, _outer_cone_angle(light._outer_cone_angle)

{
}

RenderWorldInstance::RenderWorldInstance(float4x4 const& transform)
	: _transform(transform)
{
}

 RenderWorldInstance::RenderWorldInstance(RenderWorldInstance const& rhs)
	:  _finalised(rhs._finalised)
	 , _transform(rhs._transform)
	 ,_model(rhs._model)
	 , _material_overrides(rhs._material_overrides)
{
}

RenderWorldInstance::~RenderWorldInstance()
{
}

bool RenderWorldInstance::is_ready() const
{
	return _finalised;
}

void RenderWorldInstance::finalise() 
{
	ASSERT(_model->is_loaded());
	if (!_finalised)
	{
		Model const* res = _model->get();
		_material_overrides.resize(res->get_material_count());

		for (u32 i = 0; i < res->get_material_count(); ++i)
		{
			if (_material_overrides[i])
			{
				_material_overrides[i]->bind(res->GetMaterial(i));
			}
		}


		_finalised = true;
	}

}

void RenderWorldInstance::update()
{
	Model const* res = _model->get();
	for (u32 i = 0; i < res->get_material_count(); ++i)
	{
		GetMaterialInstance(i)->update();
	}
}

void RenderWorldInstance::set_dynamic_material(u32 idx, shared_ptr<MaterialInstance> instance)
{
	_material_overrides.resize(idx + 1);
	_material_overrides[idx] = instance;
}

MaterialInstance const* RenderWorldInstance::GetMaterialInstance(u32 idx) const
{
	if (_material_overrides[idx])
	{
		return _material_overrides[idx].get();
	}

	return _model->get()->GetMaterial(idx);
}

MaterialInstance* RenderWorldInstance::GetMaterialInstance(u32 idx)
{
	if (idx < _material_overrides.size() && _material_overrides[idx])
	{
		return _material_overrides[idx].get();
	}

	return _model->get()->GetMaterial(idx);
}

VertexLayoutFlags RenderWorldInstance::GetElementUsages(u32 idx) const
{

	return _model->get()->GetElementUsages(idx);
}
