#pragma once

#include "rtti/rtti.h"

namespace framework
{
	class EntityDebugOverlay;
	class Component;

	class Entity
	{
		REFLECT(Entity);

	public:
		Entity();
		explicit Entity(float2 pos);
		explicit Entity(float3 pos);

		virtual ~Entity();

		// Helper to update the entire entities and it's component
		// #TODO: This should be managed by systems instead
		virtual void update(float dt);

		template<typename T, typename...Args>
		T* create_component( Args...args);

		Component* get_component(rtti::TypeInfo const* t) const;

		template<typename T>
		T* get_component() const;

		void set_local_position(float4 pos);
		void set_local_position(float3 pos);
		void set_local_position(float2 pos);
		void set_local_position(float x, float y);

		void set_local_scale(float3 scale);

		void set_rotation(float angle);
		void set_rotation(hlslpp::quaternion quat);

		hlslpp::quaternion const& get_rotation() const { return _rot; }

		void attach_to(Entity* parent);

		hlslpp::float4 get_local_position() const;
		hlslpp::float4 get_world_position() const;
		hlslpp::float4x4 get_local_transform() const;
		hlslpp::float4x4 get_world_transform() const;

		void set_name(std::string const& name)
		{
			_name = name;
		}

		const char* get_name() const
		{
			return _name.c_str();
		}


		friend class EntityDebugOverlay;
	protected:
		std::string _name;
		Entity* _parent;

		std::vector<Entity*> _children;
		std::vector<Component*> _components;

		hlslpp::quaternion _rot;

		hlslpp::float4 _pos;
		hlslpp::float3 _scale;
		hlslpp::float3 _rot_euler;

	};

	template<typename T, typename...Args>
	T* Entity::create_component(Args...args)
	{
		T* comp = new T(args...);
		_components.push_back(comp);

		comp->on_attach(this);

		return comp;
	}

	template<typename T>
	T* Entity::get_component() const
	{
		rtti::TypeInfo* info = rtti::Registry::template get<T>();
		return static_cast<T*>(get_component(info));
	}




}
