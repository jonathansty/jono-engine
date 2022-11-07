#pragma once

#include "core/Types.h"


#include "framework/World.h"

#ifdef ENABLE_RTTR
// Component versioning helper
#include <rttr/registration>
struct Versioning {
	static rttr::detail::metadata version(uint32_t id) {
		return rttr::detail::metadata(Versioning::get_key(), id);
	}

	static const char* get_key() { return "version"; }
};
#endif


namespace framework
{

	class EntityDebugOverlay;
	class Component;

	class ENGINE_API Entity
	{
		#ifdef ENABLE_RTTR
		RTTR_REGISTRATION_FRIEND;
		#endif
		friend class World;

	public:
		Entity();
		explicit Entity(float2 pos);
		explicit Entity(float3 pos);

		virtual ~Entity();

		// Helper to update the entire entities and it's component
		// #TODO: This should be managed by systems instead
		virtual void update(float dt);

		// Create a component from a type
		#ifdef ENABLE_RTTR
		Component* create_component(rttr::type const& t);


		Component* get_component(rttr::type const& t) const;
		#endif

		template<typename T, typename...Args>
		T* create_component( Args...args);

		void add_component(Component* component);

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

		Identifier64 get_id() const { return _id; }
		EntityHandle get_parent() const { return _parent; }
	protected:
		std::string _name;
		Identifier64 _id;
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

		WrapperQuat get_rot_euler() const {
			return { _rot };
		};
		void set_rot_euler(WrapperQuat v) {
			//_rot_euler = v;
			//_rot = hlslpp::euler({ _rot_euler.x, _rot_euler.y, _rot_euler.z });
			_rot = v.value;
		};



		hlslpp::float4 _pos;
		hlslpp::float3 _scale;

	};

	template<typename T, typename...Args>
	T* Entity::create_component(Args...args)
	{
		T* comp = new T(args...);
		_components.push_back(comp);

		comp->on_attach(this);

		return comp;
	}


	#ifdef ENABLE_RTTR
	template<typename T>
	T* Entity::get_component() const
	{
		rttr::type const& info = rttr::type::get<T>();
		return static_cast<T*>(get_component(info));
	}
	#endif




}
