#pragma once

#include "Graphics.h"
#include "core/ModelResource.h"

using RenderWorldRef = std::shared_ptr<class RenderWorld>;
using RenderWorldInstanceRef = std::shared_ptr<class RenderWorldInstance>;
using RenderWorldCameraRef = std::shared_ptr<class RenderWorldCamera>;
using RenderWorldLightRef = std::shared_ptr<class RenderWorldLight>;

class RenderWorldInstance
{
public:
	RenderWorldInstance(float4x4 const& transform)
			: _transform(transform) {}
	~RenderWorldInstance() {}

	bool is_ready() const
	{
		return _model->is_loaded();
	}

	float4x4 _transform;
	std::shared_ptr<ModelResource> _model;

	struct ConstantBufferData
	{
		float4x4 world;
		float4x4 wv;
		float4x4 wvp;
	};
	ConstantBufferRef _model_cb;

	friend class RenderWorld;
};

class RenderWorldCamera
{
public:
	enum class Projection
	{
		Ortographic,
		Perspective
	};

	struct CameraSettings
	{
		f32 aspect;
		f32 fov;
		f32 width;
		f32 height;
		f32 near_clip;
		f32 far_clip;
		Projection projection_type;
		bool reverse_z;
	};

	RenderWorldCamera();
	~RenderWorldCamera();

	// Retrieve the projection
	float4x4 const& get_proj() const
	{
		update();
		return _proj;
	}

	// Retrieve the view
	float4x4 const& get_view() const
	{
		update();
		return _view;
	}

	float4x4 const& get_world() const
	{
		update();
		return _world;
	}

	float4x4 const& get_vp() const
	{
		update();
		return _vp;
	}

	float4 get_view_direction() const
	{
		float4 world_fwd{ 0.0f, 0.0f, 1.0f, 0.0f };
		return mul(get_view(), world_fwd);
	}

	float3 get_position() const
	{
		return _position;
	}

	void set_settings(CameraSettings const& settings)
	{
		_settings = settings;
		_dirty = true;
	}
	void set_aspect(f32 aspect);

	void set_position(float3 pos)
	{
		_position = pos;
		_dirty = true;
	}

	// Conditionally sets up a target for look at.
	// This overrides any rotation the camera specifies
	void look_at(float3 target)
	{
		_target = target;
		_use_target = true;
		_dirty = true;
	}

	// Clears the lookat and falls back on the _rotation value set
	void clear_lookat()
	{
		_use_target = false;
		_dirty = true;
	}

	void update() const;

	f32 get_near() const { return _settings.near_clip; }
	f32 get_far() const { return _settings.far_clip; }
	f32 get_aspect() const { return _settings.aspect; }
	f32 get_fov() const { return _settings.fov; }
	f32 get_vertical_fov() const {

		f32 aspect = 1.0f / _settings.aspect;
		f32 fov = float1(_settings.fov);
		return float1(2.0f * atan(tan(fov / 2.0f) * aspect));
	}

protected:
	mutable bool _dirty;
	float3 _position;

	mutable bool _use_target;
	float3 _target;

	hlslpp::quaternion _rotation;
	mutable CameraSettings _settings;

	// Cache
	mutable float4x4 _proj;
	mutable float4x4 _view;
	mutable float4x4 _vp;
	mutable float4x4 _world;

	friend class RenderWorld;
};

struct CascadeInfo 
{
	float3 center;
	float4x4 view;
	float4x4 proj;
	float4x4 vp;
};

class RenderWorldLight : public RenderWorldCamera
{
public:
	enum class LightType
	{
		Directional,
		Spot,
		// #TODO
		// Point
	};

	RenderWorldLight(LightType type);
	~RenderWorldLight() {}

	bool is_directional() const { return _type == LightType::Directional; }
	bool is_spot() const { return _type == LightType::Spot; }

	float3 get_colour() const { return _colour; }
	bool get_casts_shadow() const { return _shadow_settings.casts_shadow; }

	void set_colour(float3 colour) { _colour = colour; }
	void set_casts_shadow(bool cast) { _shadow_settings.casts_shadow = true; }

	CascadeInfo const& get_cascade(u32 idx) const
	{
		return _cascade[idx];
	}

	void update_cascades(std::vector<CascadeInfo> vps)
	{
		_cascade.clear();
		for(u32 i = 0 ; i < vps.size(); ++i)
		{
			_cascade.emplace_back(vps[i]);
		}
	}

private:
	struct ShadowSettings
	{
		ShadowSettings()
				: casts_shadow(false) {}
		~ShadowSettings() {}

		bool casts_shadow;
	};

	ShadowSettings _shadow_settings;
	LightType _type;

	float3 _colour;
	std::vector<CascadeInfo> _cascade;
};

class RenderWorld final
{
public:
	using InstanceCollection = std::vector<std::shared_ptr<RenderWorldInstance>>;
	using CameraCollection = std::vector<std::shared_ptr<RenderWorldCamera>>;
	using LightCollection = std::vector<std::shared_ptr<RenderWorldLight>>;

	RenderWorld() = default;
	~RenderWorld() = default;

	void init();

	// Getters to retrieve all collections
	InstanceCollection const& get_instances() const { return _instances; }
	LightCollection const& get_lights() const { return _lights; }
	CameraCollection const& get_cameras() const { return _cameras; }

	// Retrieve individual objects for access
	std::shared_ptr<RenderWorldCamera> const& get_camera(u32 idx) const { return _cameras[0]; }
	std::shared_ptr<RenderWorldLight> const& get_light(u32 idx) const { return _lights[0]; }

	// Create render world objects
	std::shared_ptr<RenderWorldInstance> create_instance(float4x4 transform, std::string const& mesh);
	std::shared_ptr<RenderWorldCamera> create_camera();
	std::shared_ptr<RenderWorldLight> create_light(RenderWorldLight::LightType type);

	void remove_instance(std::shared_ptr<RenderWorldInstance> const& instance);

	std::shared_ptr<RenderWorldCamera> get_view_camera() const { return _cameras[_active_camera]; }

	void set_active_camera(u32 idx) { _active_camera = idx; }

private:
	u32 _active_camera = 0;

	std::mutex _instance_cs;
	InstanceCollection _instances;

	std::mutex _camera_cs;
	CameraCollection _cameras;

	std::mutex _lights_cs;
	LightCollection _lights;
};