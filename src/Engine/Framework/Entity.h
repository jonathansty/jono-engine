#pragma once

class EntityDebugOverlay;
namespace framework
{
	class Component;

	class Entity
	{
		REFLECT(Entity);

	public:
		Entity();
		explicit Entity(float2 pos);
		explicit Entity(float3 pos);

		virtual ~Entity();

		virtual void update(float dt);

		virtual void render();

		template<typename T, typename...Args>
		T* create_component( Args...args);

		Component* get_component(rtti::TypeInfo const* t) const;

		template<typename T>
		T* get_component();

		void set_local_position(float4 pos);
		void set_local_position(float3 pos);
		void set_local_position(float2 pos);
		void set_local_position(float x, float y);

		void set_local_scale(float3 scale);

		void set_rotation(float angle);
		void set_rotation(hlslpp::quaternion quat);

		void attach_to(Entity* parent) 
		{ 
			if (_parent)
			{
				_parent->_children.erase(std::find(_parent->_children.begin(), _parent->_children.end(), this));
			}
			_parent = parent; 
			_parent->_children.push_back(this);
		}

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

	protected:
		std::string _name;
		Entity* _parent;

		std::vector<Entity*> _children;
		std::vector<Component*> _components;

		hlslpp::quaternion _rot;

		hlslpp::float4 _pos;
		hlslpp::float3 _scale;
		hlslpp::float3 _rot_euler;

		friend EntityDebugOverlay;
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
	T* Entity::get_component()
	{
		rtti::TypeInfo* info = rtti::Registry::template get<T>();
		return static_cast<T*>(get_component(info));
	}




}
