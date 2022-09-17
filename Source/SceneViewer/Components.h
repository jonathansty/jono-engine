#pragma once

#include "Framework/framework.h"

#include "graphics/2DRenderContext.h"

class ModelHandle;

using hlslpp::float3;
using hlslpp::float4;
using hlslpp::float4x4;
using hlslpp::quaternion;

class SimpleMovement2D final : public framework::Component
{
	RTTR_ENABLE(framework::Component);

public:
	SimpleMovement2D();

	SimpleMovement2D(float2 pos, float speed);

	~SimpleMovement2D();

	void on_attach(framework::Entity* ent) override;
	void update(float dt) override;

	void set_speed(float speed) { _speed = speed; }

	void reset() { _speed = 0.0; }

	float4 _offset;
	float _elapsed = 0.0;
	float _speed;
};

class SimpleMovement3D final : public framework::Component
{
	RTTR_ENABLE(framework::Component);

public:
	SimpleMovement3D() = default;

	~SimpleMovement3D() = default;

	void update(float dt)
	{
		_elapsed += dt * _speed;
		framework::Entity* ent = get_entity();
		float3 up{ 0.0f, 1.0f, 0.0f };

		quaternion rot = ent->get_rotation();
		quaternion added = quaternion::rotation_axis(up, hlslpp::radians(hlslpp::float1(dt * _speed)));
		rot = hlslpp::mul(rot, added);
		ent->set_rotation(rot);
		//ent->set_local_position(_offset.x + cos(_elapsed) * 100.0, _offset.y + sin(_elapsed) * 100.0);
	}

	void set_speed(float speed) { _speed = speed; }

	float _elapsed = 0.0f;
	float _speed = 0.0f;
};

#if FEATURE_D2D
class BitmapComponent final : public framework::Component
{
	RTTR_ENABLE(framework::Component);

public:
	BitmapComponent()
			: Component()
	{
	}

	BitmapComponent(std::string const& path)
			: framework::Component()
	{
		_bmp = Bitmap::load(path);
	}
	~BitmapComponent()
	{
	}

	void render()
	{
		GameEngine::instance()->_d2d_ctx->draw_bitmap(_bmp.get(), (int)(-_bmp->get_width() / 2.0), (int)(-_bmp->get_height() / 2.0));
	}

	unique_ptr<Bitmap> _bmp;
};
#endif

class ModelComponent final : public framework::Component
{
	RTTR_ENABLE(framework::Component);

public:
	using Entity = framework::Entity;

	ModelComponent();
	virtual ~ModelComponent();

	virtual void on_attach(Entity* ent) override;
	virtual void on_detach(Entity* ent) override;

	bool is_loaded() const;
	void set_model_path(std::string const& mesh);

	std::string const& get_model_path();

private:
	std::string _model_path;

	RenderWorldInstanceRef _instance;

};

class LightComponent final : public framework::Component
{
	RTTR_ENABLE(framework::Component);
	RTTR_REGISTRATION_FRIEND;

public:
	using Entity = framework::Entity;

	LightComponent()
			: _color({ float3(0.0, 0.0, 0.0) })
			, _intensity(0.0)
	{
	}

	virtual ~LightComponent() {}

	hlslpp::float3 get_color() { return _color; }

private:
	Helpers::WrapperFloat3 _color;
	float _intensity;
};

class CameraComponent final : public framework::Component
{
	RTTR_ENABLE(framework::Component);
	RTTR_REGISTRATION_FRIEND;

public:
	using Entity = framework::Entity;

	CameraComponent();
	~CameraComponent();

	CameraComponent& operator=(CameraComponent const&) = delete;

	void update(float dt) override;

	void look_at(float3 eye, float3 target, float3 up = { 0.0, 0.0f, 1.0f });

	float get_fov() const { return _fov; }
	float get_near_plane() const { return _near_plane; }
	float get_far_plane() const { return _far_plane; }

	float set_fov(float fov)
	{
		assert(fov > 0.01f && fov < 180.0f);
		_fov = fov;
	}

	float set_planes(float n, float f)
	{
		_near_plane = n;
		_far_plane = f;
	}

	static const float DEFAULT_FOV;

private:
	float _fly_speed;
	float _fov;
	float _near_plane;
	float _far_plane;

	float _x_angle;
	float _y_angle;

	hlslpp::float2 _prev_position;
};
