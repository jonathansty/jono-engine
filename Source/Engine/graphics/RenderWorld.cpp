#include "engine.pch.h"

#include "GameEngine.h"
#include "RenderWorld.h"

void RenderWorld::init() {
	_instances.reserve(100);
	_cameras.reserve(10);
	_lights.reserve(10);
}

std::shared_ptr<RenderWorldInstance> RenderWorld::create_instance(float4x4 transform, std::string const& mesh) {

	std::lock_guard l{ _instance_cs };
	std::shared_ptr<RenderWorldInstance> inst = std::make_shared<RenderWorldInstance>(transform);
	inst->_model_cb = ConstantBuffer::create(Graphics::GetDevice().Get(), sizeof(RenderWorldInstance::ConstantBufferData), true, ConstantBuffer::BufferUsage::Dynamic, nullptr);

	ModelResource::init_parameters params{};
	params.path = mesh;
	inst->_mesh = ResourceLoader::instance()->load<ModelResource>(params, false);

	_instances.push_back(inst);

	return inst;
}

std::shared_ptr<RenderWorldCamera> RenderWorld::create_camera() {
	std::lock_guard l{ _camera_cs };
	auto cam = std::make_shared<RenderWorldCamera>();
	_cameras.push_back(cam);
	return cam;
}

std::shared_ptr<RenderWorldLight> RenderWorld::create_light(RenderWorldLight::LightType type) {
	std::lock_guard l{ _lights_cs };
	auto result = make_shared<RenderWorldLight>(type);
	_lights.push_back(result);
	return result;
}

RenderWorldCamera::RenderWorldCamera()
		: _dirty(true)
		, _position()
		, _rotation(hlslpp::quaternion::identity())
		, _proj()
		, _view()
		, _settings()
		,_use_target(false)
{
}

RenderWorldCamera::~RenderWorldCamera() {
}

void RenderWorldCamera::update() const {
	if (_dirty) {
		auto t = float4x4::translation(_position);
		auto r = float4x4(_rotation);
		_world = hlslpp::mul(r, t);
		_view = hlslpp::inverse(_world);

		if (_use_target){
			_view = float4x4::look_at(_position, _target, float3(0.0f, 1.0f, 0.0));
			_world = hlslpp::inverse(_view);
			_use_target = false;
		}

		bool reverse_z = _settings.reverse_z;
		f32 near_clip = reverse_z ? _settings.far_clip : _settings.near_clip;
		f32 far_clip = reverse_z ? _settings.near_clip : _settings.far_clip;

		hlslpp::projection proj(hlslpp::frustum::field_of_view_x(hlslpp::radians(float1(_settings.fov)), _settings.aspect, near_clip, far_clip), hlslpp::zclip::zero);
		if(_settings.projection_type == Projection::Perspective) {
			_proj = float4x4::perspective(proj);
		} else if(_settings.projection_type == Projection::Ortographic) {
			_proj = float4x4::orthographic(proj);
		}

		_vp = hlslpp::mul(_view, _proj);

		_dirty = false;
	}
}

RenderWorldLight::RenderWorldLight(LightType type)
		: _type(type), _colour(1.0f, 1.0f, 1.0f), _shadow_settings() {
}
