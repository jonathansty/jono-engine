#include "engine.pch.h"

#include "GameEngine.h"
#include "RenderWorld.h"

void RenderWorld::init()
{
	_instances.reserve(100);
	_cameras.reserve(10);
	_lights.reserve(10);
}

std::shared_ptr<RenderWorldInstance> RenderWorld::create_instance(float4x4 transform, std::string const& mesh)
{
	std::lock_guard l{ _instance_cs };
	std::shared_ptr<RenderWorldInstance> inst = std::make_shared<RenderWorldInstance>(transform);
	inst->_model_cb = ConstantBuffer::create(Graphics::get_device().Get(), sizeof(RenderWorldInstance::ConstantBufferData), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);

	ModelResource::init_parameters params{};
	params.path = mesh;
	inst->_model = ResourceLoader::instance()->load<ModelResource>(params, false, false);

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
	auto it = std::find(_instances.begin(), _instances.end(), instance);
	if (it != _instances.end())
	{
		_instances.erase(it);
	}
}

RenderWorldCamera::RenderWorldCamera()
		: _dirty(true)
		, _position()
		, _rotation(hlslpp::quaternion::identity())
		, _proj()
		, _view()
		, _settings()
		, _use_target(false)
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
		auto t = float4x4::translation(_position);
		auto r = float4x4(_rotation);

		_world = hlslpp::mul(r, t);
		_view = hlslpp::inverse(_world);

		if (_use_target)
		{
			_view = float4x4::look_at(_position, _target, float3(0.0f, 1.0f, 0.0));
			_world = hlslpp::inverse(_view);
		}

		bool reverse_z = _settings.reverse_z;
		f32 near_clip = _settings.near_clip;
		f32 far_clip = _settings.far_clip;

		if (_settings.projection_type == Projection::Perspective)
		{
			hlslpp::projection proj(hlslpp::frustum::field_of_view_x(_settings.fov, _settings.aspect, near_clip, far_clip), reverse_z ? hlslpp::zclip::minus_one : hlslpp::zclip::zero);
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
{
}
