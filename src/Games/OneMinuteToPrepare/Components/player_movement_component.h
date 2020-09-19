#pragma once

#include <Engine.hpp>

using namespace framework;

class rigid_body_2d final : public framework::Component
{
	REFLECT(rigid_body_2d);
public:
	rigid_body_2d(BodyType type = BodyType::DYNAMIC) : _type(type) {};
	~rigid_body_2d() {};

	std::shared_ptr<PhysicsActor> get_actor() const { return _actor; }

	virtual void update(float dt);

	virtual void on_attach(Entity* _ent) override;

	virtual void on_detach(Entity* _ent) override;
private:
	BodyType _type;
	std::shared_ptr<PhysicsActor> _actor;
};

class player_movement_component final : public framework::Component
{
	REFLECT(player_movement_component);

public:	
	player_movement_component() 
		: speed(100.0f)
	{
	};
	~player_movement_component() {};

private:
	virtual void update(float dt);
	virtual void render();


public:
	float speed;

};

