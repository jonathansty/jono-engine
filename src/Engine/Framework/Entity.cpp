#include <stdafx.h>
#include "Entity.h"

#include "Component.h"

using namespace framework;
IMPL_REFLECT(Entity)
{
	type.register_property("name", &Entity::_name);
	type.register_property("position", &Entity::_pos);
	type.register_property<Entity, XMFLOAT3>("rotation", 
		[](Entity* obj, XMFLOAT3 const* v) { 
			obj->_rot_euler = *v; 
			obj->_rot = XMQuaternionRotationRollPitchYaw(v->x, v->y, v->z);
		}, 
		[](Entity* obj, XMFLOAT3** out) 
		{
			*out = &obj->_rot_euler;
		}
	);
}

Entity::Entity(XMFLOAT3 pos)
	: _parent(nullptr)
	, _pos(pos.x, pos.y, pos.z)
	, _scale({ 1.0f, 1.0f, 1.0f })
	, _rot(XMQuaternionIdentity())
	, _rot_euler({0.0f,0.0f,0.0f})
{
}

Entity::Entity() : Entity({ 0.0f, 0.0f })
{

}

Entity::Entity(XMFLOAT2 pos) : Entity({pos.x, pos.y, 0.0})
{

}

void Entity::set_local_position(XMFLOAT3 pos)
{
	_pos = pos;
}

void Entity::set_local_scale(XMFLOAT3 scale)
{
	_scale = scale;
}

void Entity::set_rotation(XMVECTOR quat)
{
	XMMATRIX m = XMMatrixRotationQuaternion(quat);
	XMVECTOR scale, rot, trans;
	XMMatrixDecompose(&scale, &rot, &trans, m);

	XMStoreFloat3(&_rot_euler, rot);

	_rot = quat;
}

void Entity::set_rotation(float angle)
{
	set_rotation(XMQuaternionRotationRollPitchYaw(0.0, 0.0, angle));
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
	XMMATRIX scaleMat = XMMatrixScaling(_scale.x, _scale.y, _scale.z);

	return XMMatrixMultiply(rotMat, XMMatrixMultiply(transMat, scaleMat));
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
		comp->on_detach(this);
		delete comp;
	}
	_components.clear();

}

void Entity::update(float dt)
{
	for (Component* el : _components)
	{
		if(el->is_active())
			el->update(dt);
	}
}

void Entity::render()
{
	for (Component* el : _components)
	{
		if(el->is_active())
			el->render();
	}
}

void Entity::set_local_position(XMFLOAT2 pos)
{
	_pos = { pos.x, pos.y, 0.0 };
}

void Entity::set_local_position(float x, float y)
{
	set_local_position({ x,y });
}

XMFLOAT3 Entity::get_local_position() const
{
	return _pos;
}

Component* Entity::get_component(rtti::TypeInfo const* t) const
{
	for (Component* c : _components)
	{
		if (c->get_type()->is(t))
			return c;
	}

	return nullptr;
}
