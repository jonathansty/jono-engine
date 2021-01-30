#pragma once

#include "RTTI/rtti.h"
#include "Framework/framework.h"

#include "graphics/2DRenderContext.h"

class ModelResource;

using hlslpp::float3;
using hlslpp::float4;
using hlslpp::float4x4;
using hlslpp::quaternion;

class SimpleMovement2D final : public framework::Component
{
	REFLECT(SimpleMovement2D)
public:
	SimpleMovement2D();

	SimpleMovement2D(XMFLOAT2 pos, float speed);

	~SimpleMovement2D();

	void on_attach(framework::Entity* ent) override;
	void update(float dt) override;

	void set_speed(float speed) { _speed = speed; }

	void reset() { _speed = 0.0; }

	float4 _offset;
	float _elapsed = 0.0;
	float _speed;

};

class SimpleMovement3D final : public framework::Component {
	REFLECT(SimpleMovement3D)
public:
	SimpleMovement3D() = default;

	~SimpleMovement3D() = default;

	void update(float dt) {
		_elapsed += dt * _speed;
		framework::Entity* ent = get_entity();
		float3 up{ 0.0f, 1.0f, 0.0f };

		quaternion rot = ent->get_rotation();
		quaternion added = hlslpp::axisangle(up, hlslpp::radians(hlslpp::float1(dt * _speed)));
		rot *= added;
		ent->set_rotation(rot);
		//ent->set_local_position(_offset.x + cos(_elapsed) * 100.0, _offset.y + sin(_elapsed) * 100.0);
	}

	void set_speed(float speed) { _speed = speed; }

	float _elapsed = 0.0f;
	float _speed = 0.0f;
};


class BitmapComponent final : public framework::Component
{
	REFLECT(BitmapComponent)
public:
	BitmapComponent() : Component()
	{

	}

	BitmapComponent(std::string const& path)
		: framework::Component()
	{
		_bmp = new Bitmap(String(path.c_str()));
	}
	~BitmapComponent()
	{
		delete _bmp;
	}

	void render()
	{
		GameEngine::instance()->_d2d_ctx->draw_bitmap(_bmp, -_bmp->GetWidth()/2.0, -_bmp->GetHeight() / 2.0);
	}


	Bitmap* _bmp;

};

class SimpleMeshComponent final : public framework::Component
{
	REFLECT(SimpleMeshComponent);

public:
	using Entity = framework::Entity;

	SimpleMeshComponent();;
	virtual ~SimpleMeshComponent();;

	virtual void on_attach(Entity* ent) override;
	virtual void on_detach(Entity* ent) override;

	virtual void render() override;

	bool is_loaded() const;
	void set_model(std::string const& mesh);

	std::shared_ptr<ModelResource> get_model_resource() const { return _resource; };

private:
	std::shared_ptr<ModelResource> _resource;

};

class LightComponent final : public framework::Component
{
	REFLECT(LightComponent);
public:
	using Entity = framework::Entity;

};

class CameraComponent final : public framework::Component
{
	REFLECT(CameraComponent);
public:
	using Entity = framework::Entity;

	CameraComponent();
	~CameraComponent();

	CameraComponent(CameraComponent const&) = delete;
	CameraComponent& operator=(CameraComponent const&) = delete;

	void update(float dt) override;

	void look_at(float3 eye, float3 target, float3 up = { 0.0,0.0f,1.0f });

	float get_fov() const { return _fov; }
	float get_near_plane() const { return _near_plane; }
	float get_far_plane() const { return _far_plane; }

	float set_fov(float fov) { assert(fov > 0.01f && fov < 180.0f); _fov = fov; }
	float set_planes(float n, float f) { _near_plane = n; _far_plane = f; }

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


