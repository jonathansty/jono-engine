#include <stdafx.h>
#include "Entity.h"

#include "Component.h"
#include "core/Math.h"

using hlslpp::float3;
using hlslpp::float4;
using hlslpp::float4x4;
using hlslpp::quaternion;
using hlslpp::float1;

using namespace framework;
IMPL_REFLECT(Entity)
{
	type.register_property("name", &Entity::_name);
	type.register_property("position", &Entity::_pos);
	type.register_property<Entity, float3>("rotation", 
		[](Entity* obj, float3 const* v) { 
			obj->_rot_euler = *v; 
			obj->_rot = hlslpp::euler({ v->x, v->y, v->z });
		}, 
		[](Entity* obj, float3** out) 
		{
			*out = &obj->_rot_euler;
		}
	);
}

Entity::Entity(float3 pos)
	: _parent(nullptr)
	, _pos(pos.x, pos.y, pos.z, 1.0f)
	, _scale(float3{ 1.0f, 1.0f, 1.0f })
	, _rot(hlslpp::quaternion::identity())
	, _rot_euler(float3{0.0f,0.0f,0.0f})
{
}

Entity::Entity() : Entity(float2{ 0.0f, 0.0f })
{

}

Entity::Entity(float2 pos) : Entity({pos.x, pos.y, 0.0})
{

}

void Entity::set_local_position(float4 pos) {
	_pos = pos;
}
void Entity::set_local_position(float3 pos)
{
	_pos = float4(pos,1.0f);
}

void Entity::set_local_scale(float3 scale)
{
	_scale = scale;
}

float3 to_euler(hlslpp::quaternion q) {
	// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	float3 angles;

	// roll (x-axis rotation)
	float1 sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float1 cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = hlslpp::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	float1 sinp = 2 * (q.w * q.y - q.z * q.x);
	if (hlslpp::all(hlslpp::abs(sinp) >= float1(1)))
		angles.y = std::copysign(M_PI / 2, float(sinp)); // use 90 degrees if out of range
	else
		angles.y = hlslpp::asin(sinp);

	// yaw (z-axis rotation)
	float1 siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float1 cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = hlslpp::atan2(siny_cosp, cosy_cosp);

	return angles;
}

void Entity::set_rotation(hlslpp::quaternion quat)
{
	_rot = quat;
	_rot_euler = to_euler(_rot);
}

void Entity::set_rotation(float angle)
{
	set_rotation(hlslpp::euler({ 0.0, 0.0, angle }));
}

using hlslpp::float4x4;
using hlslpp::float3;
float4x4 Entity::get_world_transform() const
{
	float4x4 curr = get_local_transform();
	if (_parent)
	{
		curr = hlslpp::mul(curr, _parent->get_world_transform());
	}

	return curr;
}

float4x4 Entity::get_local_transform() const
{
	float4x4 m = float4x4::identity();

	float4 pos = _pos;
	float4x4 transMat = float4x4::translation(pos.xyz);
	float4x4 rotMat = float4x4(_rot);
	float4x4 scaleMat = float4x4::scale(_scale.x, _scale.y, _scale.z);

	return hlslpp::mul(rotMat, hlslpp::mul(scaleMat,transMat));
}

hlslpp::float4 Entity::get_world_position() const
{
	float4x4 world = get_world_transform();
	return hlslpp::mul(world, hlslpp::float4(_pos));
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
	float4x4 transform = get_world_transform();

	float3x3 c = float3x3(transform._11_12_14, transform._21_22_24, transform._41_42_44);
	GameEngine::instance()->_d2d_ctx->set_world_matrix(c);

	for (Component* el : _components)
	{
		if(el->is_active())
			el->render();
	}
}

void Entity::set_local_position(float2 pos)
{
	_pos = { pos.x, pos.y, 0.0, 1.0f };
}

void Entity::set_local_position(float x, float y)
{
	set_local_position(float2{ x,y });
}

float4 Entity::get_local_position() const
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
