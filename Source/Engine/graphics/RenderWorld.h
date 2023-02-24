#pragma once

#include "Graphics.h"
#include "ConstantBuffer.h"
#include "core/ModelResource.h"

using RenderWorldRef = std::shared_ptr<class RenderWorld>;
using RenderWorldInstanceRef = std::shared_ptr<class RenderWorldInstance>;
using RenderWorldCameraRef = std::shared_ptr<class RenderWorldCamera>;
using RenderWorldLightRef = std::shared_ptr<class RenderWorldLight>;

// Render world 'model' instance
class ENGINE_API RenderWorldInstance
{
public:
	RenderWorldInstance(float4x4 const& transform);
	RenderWorldInstance(RenderWorldInstance const& rhs);

	~RenderWorldInstance();

	bool is_ready() const;

	bool is_finalised() const { return _finalised; }
	void finalise();
	void update();

	void set_dynamic_material(u32 idx, shared_ptr<MaterialInstance> instance);

	MaterialInstance const* GetMaterialInstance(u32 idx) const;
	MaterialInstance* GetMaterialInstance(u32 idx);

	VertexLayoutFlags GetElementUsages(u32 idx) const;

	bool _finalised = false;
	float4x4 _transform;

	// Reference to a model
	std::shared_ptr<ModelHandle> _model;

	std::vector<std::shared_ptr<MaterialInstance>> _material_overrides;

	friend class RenderWorld;
};

class ENGINE_API RenderWorldCamera
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
	RenderWorldCamera(RenderWorldCamera const&);;

	RenderWorldCamera& operator=(RenderWorldCamera const& rhs)
	{
		this->_dirty = true;
		this->_proj = rhs._proj;
		this->_world = rhs._world;
		this->_view = rhs._view;
		this->_settings = rhs._settings;

		return *this;
	}
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
		return float3(_world.f32_128_3[0], _world.f32_128_3[1], _world.f32_128_3[2]);
	}

	void set_settings(CameraSettings const& settings)
	{
		_settings = settings;
		_dirty = true;
	}
	void set_aspect(f32 aspect);

	void set_view(float4x4 view)
	{
		_view = view;
		_world = hlslpp::inverse(view);
	}
	void set_world(float4x4 world)
	{
		_world = world;
		_view = hlslpp::inverse(world);
	}


	void update() const;

	f32 get_near() const { return _settings.near_clip; }
	f32 get_far() const { return _settings.far_clip; }
	f32 get_aspect() const { return _settings.aspect; }

	f32 get_horizontal_fov() const 
	{ 
		f32 aspect = _settings.aspect;
		f32 fov = float1(_settings.fov);
		return float1(2.0f * atan(tan(fov / 2.0f) * aspect));
	}

	f32 get_vertical_fov() const 
	{
		return _settings.fov;
	}

protected:
	float4x4 _view;
	float4x4 _world;

	mutable bool _dirty;
	mutable CameraSettings _settings;

	// Cache
	mutable float4x4 _proj;
	mutable float4x4 _vp;

	friend class RenderWorld;
};

struct CascadeInfo 
{
	float3 center;
	float4x4 view;
	float4x4 proj;
	float4x4 vp;
};

class ENGINE_API RenderWorldLight : public RenderWorldCamera
{
public:
	enum class LightType
	{
		Directional,
		Spot,
		Point
	};

	RenderWorldLight(LightType type);
	RenderWorldLight(RenderWorldLight const& light);
	RenderWorldLight& operator=(RenderWorldLight const&) = delete;
	~RenderWorldLight() {}

	bool is_directional() const { return _type == LightType::Directional; }
	bool is_spot() const { return _type == LightType::Spot; }
	LightType get_type() const { return _type; }

	f32 get_range() const { return _range; }
	float3 get_colour() const { return _colour; }
	bool get_casts_shadow() const { return _shadow_settings.casts_shadow; }
	f32 get_cone_angle() const { return _cone_angle; }
	f32 get_outer_cone_angle() const { return _outer_cone_angle; }

	void set_colour(float3 colour) { _colour = colour; }
	void set_casts_shadow(bool cast) { _shadow_settings.casts_shadow = true; }
	void set_range(f32 range) { _range = range; }
	void set_cone_angle(f32 cone_angle) { _cone_angle = cone_angle; }
	void set_outer_cone_angle(f32 outer_cone_angle) { _outer_cone_angle = outer_cone_angle; }

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
		ShadowSettings() : casts_shadow(false) {}
		~ShadowSettings() {}

		bool casts_shadow;
	};

	ShadowSettings _shadow_settings;

	// Only used for directional lights
	std::vector<CascadeInfo> _cascade;

	LightType _type;
	float3 _colour;
	f32 _range = 0.0f;
	f32 _cone_angle = 0.0f;
	f32 _outer_cone_angle = 0.0f;
};

class ENGINE_API RenderWorld final
{
public:
	using InstanceCollection = std::vector<std::shared_ptr<RenderWorldInstance>>;
	using CameraCollection = std::vector<std::shared_ptr<RenderWorldCamera>>;
	using LightCollection = std::vector<std::shared_ptr<RenderWorldLight>>;

	RenderWorld() = default;
	~RenderWorld() = default;

	RenderWorld(RenderWorld const& rhs);
	RenderWorld& operator=(RenderWorld const& rhs);

	static constexpr u32 c_instance_reserve = 512;
	static constexpr u32 c_light_reserve = 10;
	static constexpr u32 c_camera_reserve = 1;

	// Initializes the render world to reserve space for N amount of data (See constants above)
	void Init();
    void Clear();

	// Getters to retrieve all collections
	InstanceCollection const& get_instances() const { return _instances; }
	LightCollection    const& get_lights()    const { return _lights; }
	CameraCollection   const& get_cameras()   const { return _cameras; }

	// Retrieve individual objects for access
	std::shared_ptr<RenderWorldCamera> const& get_camera(u32 idx) const { return _cameras[idx]; }
	std::shared_ptr<RenderWorldLight>  const& get_light(u32 idx) const { return _lights[idx]; }

	// Create render world objects
	std::shared_ptr<RenderWorldInstance> create_instance(float4x4 transform, std::string const& mesh);
	std::shared_ptr<RenderWorldCamera>   create_camera();
	std::shared_ptr<RenderWorldLight>    create_light(RenderWorldLight::LightType type);

	void remove_instance(std::shared_ptr<RenderWorldInstance> const& instance);
	void remove_light(std::shared_ptr<RenderWorldLight> const& light);

	// Returns the current active view camera
	std::shared_ptr<RenderWorldCamera> get_view_camera() const;

	// Updates the current active view camera based on the index in the list
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