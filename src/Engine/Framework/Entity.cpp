#include "engine.stdafx.h"
#include "Entity.h"

#include "Component.h"
#include "core/Math.h"

#include <rttr/registration>

using hlslpp::float3;
using hlslpp::float4;
using hlslpp::float4x4;
using hlslpp::quaternion;
using hlslpp::float1;


RTTR_REGISTRATION{

	using namespace rttr;
	using namespace framework; 

	registration::class_<WrapperFloat4>("float4");
	registration::class_<WrapperFloat3>("float3");


	registration::class_<Entity>("Entity")
			.property("name", &Entity::_name)
			.property("position", &Entity::get_pos, &Entity::set_pos)
			.property("rotation", &Entity::get_rot_euler, &Entity::set_rot_euler)
			.property("scale", &Entity::get_scale, &Entity::set_scale);
}

namespace framework {

//IMPL_REFLECT(Entity) {
//	type.register_property("name", &Entity::_name);
//	type.register_property("position", &Entity::_pos);
//
//	// Special property with conversion functions
//	type.register_property<Entity, float3>(
//			"rotation",
//			[](Entity* obj, float3 const* v) {
//				obj->_rot_euler = *v;
//				obj->_rot = hlslpp::euler({ v->x, v->y, v->z });
//			},
//			[](Entity* obj, float3** out) {
//				*out = &obj->_rot_euler;
//			});
//}

Entity::Entity(float3 pos)
		: _parent(nullptr)
		, _pos(pos.x, pos.y, pos.z, 1.0f)
		, _scale(float3{ 1.0f, 1.0f, 1.0f })
		, _rot(hlslpp::quaternion::identity())
		, _rot_euler(float3{ 0.0f, 0.0f, 0.0f }) {
}

Entity::Entity()
		: Entity(float2{ 0.0f, 0.0f }) {
}

Entity::Entity(float2 pos)
		: Entity({ pos.x, pos.y, 0.0 }) {
}

void Entity::set_local_position(float4 pos) {
	_pos = pos;
}
void Entity::set_local_position(float3 pos) {
	_pos = float4(pos, 1.0f);
}

void Entity::set_local_scale(float3 scale) {
	_scale = scale;
}

void Entity::set_rotation(hlslpp::quaternion quat) {
	_rot = quat;
	_rot_euler = hlslpp_helpers::to_euler(_rot);
}

void Entity::set_rotation(float angle) {
	set_rotation(hlslpp::euler({ 0.0, 0.0, angle }));
}

using hlslpp::float3;
using hlslpp::float4x4;
float4x4 Entity::get_world_transform() const {
	float4x4 curr = get_local_transform();
	if (_parent) {
		curr = hlslpp::mul(curr, _parent->get_world_transform());
	}

	return curr;
}

float4x4 Entity::get_local_transform() const {
	float4x4 m = float4x4::identity();

	float4 pos = _pos;
	float4x4 transMat = float4x4::translation(pos.xyz);
	float4x4 rotMat = float4x4(_rot);
	float4x4 scaleMat = float4x4::scale(_scale.x, _scale.y, _scale.z);

	return hlslpp::mul(rotMat, hlslpp::mul(scaleMat, transMat));
}

hlslpp::float4 Entity::get_world_position() const {
	float4x4 world = get_world_transform();
	return hlslpp::mul(world, hlslpp::float4(_pos));
}

Entity::~Entity() {
	for (auto comp : _components) {
		comp->on_detach(this);
		delete comp;
	}
	_components.clear();
}

void Entity::update(float dt) {
	for (Component* el : _components) {
		if (el->is_active()) {
			el->update(dt);
		}
	}
}

void Entity::set_local_position(float2 pos) {
	_pos = { pos.x, pos.y, 0.0, 1.0f };
}

void Entity::set_local_position(float x, float y) {
	set_local_position(float2{ x, y });
}

float4 Entity::get_local_position() const {
	return _pos;
}

Component* Entity::get_component(rttr::type const& t) const {
	for (Component* c : _components) {
		rttr::type info = rttr::type::get(c);
		if (info == t) {
			return c;
		}
	}

	return nullptr;
}

void framework::Entity::attach_to(Entity* parent) {
	if (_parent) {
		_parent->_children.erase(std::find(_parent->_children.begin(), _parent->_children.end(), this));
	}
	_parent = parent;
	_parent->_children.push_back(this);
}
} // namespace framework
