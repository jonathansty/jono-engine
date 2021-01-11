#pragma once

#include "RTTI/rtti.h"
#include "Framework/framework.h"

class ModelResource;

class SimpleMovement final : public framework::Component
{
	REFLECT(SimpleMovement)
public:
	SimpleMovement();

	SimpleMovement(XMFLOAT2 pos, float speed);

	~SimpleMovement();

	void on_attach(framework::Entity* ent) override;
	void update(float dt) override;

	void set_speed(float speed) { _speed = speed; }

	XMFLOAT3 _offset;
	float _elapsed = 0.0;
	float _speed;

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
		XMFLOAT3 pos = get_entity()->get_world_position();
		GameEngine::instance()->DrawBitmap(_bmp, (int)pos.x, (int)pos.y);
	}


private:
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


	virtual void update(float dt) override;
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

	void look_at(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up = { 0.0,0.0f,1.0f });

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
	
	XMFLOAT2 _prev_position;

};


