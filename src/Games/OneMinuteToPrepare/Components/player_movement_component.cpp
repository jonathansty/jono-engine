#include "player_movement_component.h"

#include <Engine.hpp>
IMPL_REFLECT(player_movement_component)
{
	type.bind_parent<Component>();
}

IMPL_REFLECT(rigid_body_2d)
{
	type.bind_parent<Component>();
}

// Keybinds
static const uint64_t s_up = VK_UP;
static const uint64_t s_down = VK_DOWN;
static const uint64_t s_right = VK_RIGHT;
static const uint64_t s_left = VK_LEFT;

void player_movement_component::update(float dt)
{
	XMFLOAT3 pos = get_entity()->get_local_position();

	auto physics_component = get_entity()->get_component<rigid_body_2d>();
	std::shared_ptr<PhysicsActor> actor = physics_component->get_actor();

	if (game_engine::instance()->IsKeyboardKeyDown(s_up))
	{
		pos.y -= speed * dt;
	}

	if (game_engine::instance()->IsKeyboardKeyDown(s_down))
	{
		pos.y += speed * dt;
	}

	if (game_engine::instance()->IsKeyboardKeyDown(s_right))
	{
		pos.x += speed * dt;
	}

	if (game_engine::instance()->IsKeyboardKeyDown(s_left))
	{
		pos.x -= speed * dt;
	}

	actor->SetPosition({ pos.x, pos.y });
}

void player_movement_component::render()
{
	XMFLOAT3 world_pos = this->get_entity()->get_world_position();

	game_engine::instance()->set_color(COLOR{ 255,255,255 });
	game_engine::instance()->FillEllipse(world_pos.x, world_pos.y, 10, 10);

}

void rigid_body_2d::on_attach(Entity* _ent)
{
	__super::on_attach(_ent);

	_actor = std::make_shared<PhysicsActor>(DOUBLE2(0.0, 0.0), 0.0, _type);
}

void rigid_body_2d::on_detach(Entity* _ent)
{
	__super::on_detach(_ent);

	_actor.reset();
}

void rigid_body_2d::update(float dt)
{
	this->get_entity()->set_local_position(get_actor()->GetPosition().x, get_actor()->GetPosition().y);

}
