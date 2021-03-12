#pragma once

#include "Structs.h"

#include <rttr/registration>

#include "framework/World.h"


namespace framework
{

	class EntityDebugOverlay;
	class Component;

	class Entity
	{
		RTTR_REGISTRATION_FRIEND;
		friend class World;

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

		Component* get_component(rttr::type const& t) const;

		std::vector<Component*> get_components() const { return _components; }

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
		EntityHandle _parent;

		std::vector<EntityHandle> _children;
		std::vector<Component*> _components;

		hlslpp::quaternion _rot;

		WrapperFloat4 get_pos() {
			return { _pos };
		};

		void set_pos(WrapperFloat4 pos) {
			_pos = pos;
		};

		WrapperFloat3 get_scale() const {
			return { _scale };
		};

		void set_scale(WrapperFloat3 v) {
			_scale = v;
		};

		WrapperFloat3 get_rot_euler() const {
			return { _rot_euler };
		};
		void set_rot_euler(WrapperFloat3 v) {
			_rot_euler = v;
			_rot = hlslpp::euler({ _rot_euler.x, _rot_euler.y, _rot_euler.z });
		};



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
		rttr::type const& info = rttr::type::get<T>();
		return static_cast<T*>(get_component(info));
	}




}
