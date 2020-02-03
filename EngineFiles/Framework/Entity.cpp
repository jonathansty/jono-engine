#include "stdafx.h"
#include "Entity.h"

#include "Component.h"

using namespace framework;
IMPL_REFLECT(Entity)
{
	type.register_property("position", &Entity::_pos);
	type.register_property("rotation", &Entity::_rot);
}

Entity::Entity(XMFLOAT2 pos)
	: _pos(pos.x, pos.y, 0.0)
{

}

Entity::Entity()
	: _pos(0.0,0.0,0.0)
{

}

framework::Entity::Entity(XMFLOAT3 pos)
	: _pos(pos)
{

}

void Entity::set_rotation(float angle)
{
	_rot = XMQuaternionRotationRollPitchYaw(0.0, 0.0, angle);
}

XMMATRIX Entity::get_world_transform() const
{
	XMMATRIX curr = get_local_transform();
	if (_parent)
	{
		curr = XMMatrixMultiply(get_local_transform(), _parent->get_world_transform());
	}

	return curr;
}

XMMATRIX Entity::get_local_transform() const
{
	XMMATRIX m = XMMatrixIdentity();

	XMVECTOR pos = XMLoadFloat3(&_pos);

	XMMATRIX transMat = XMMatrixTranslationFromVector(pos);
	XMMATRIX rotMat = XMMatrixRotationQuaternion(_rot);

	return XMMatrixMultiply(rotMat, transMat);
}

XMFLOAT3 Entity::get_world_position() const
{
	XMVECTOR scale, pos, quat;
	XMMatrixDecompose(&scale, &quat, &pos, get_world_transform());
	XMFLOAT3 out{};
	XMStoreFloat3(&out, pos);

	return out;
}

Entity::~Entity()
{
	for (auto comp : _components)
	{
		delete comp;
	}
	_components.clear();

}

void Entity::update(float dt)
{
	for (Component* el : _components)
	{
		el->update(dt);
	}
}

void Entity::render()
{
	for (Component* el : _components)
	{
		el->render();
	}
}

void Entity::set_position(XMFLOAT2 pos)
{
	_pos = { pos.x, pos.y, 0.0 };
}

void Entity::set_position(float x, float y)
{
	set_position({ x,y });
}

XMFLOAT3 Entity::get_local_position() const
{
	return _pos;
}

Component* Entity::get_component(rtti::TypeInfo* t) const
{
	for (Component* c : _components)
	{
		if (c->get_type()->is(t))
			return c;
	}

	return nullptr;
}
