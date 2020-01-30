#pragma once

namespace framework
{
	class Component;

	class Entity
	{
		REFLECT(Entity);

	public:
		Entity();
		Entity(XMFLOAT2 pos);
		Entity(XMFLOAT3 pos);

		virtual ~Entity();

		virtual void update(float dt);

		virtual void render();

		template<typename T, typename...Args>
		T* create_component(std::string const& name, Args...args);

		Component* get_component(rtti::TypeInfo* t) const;

		template<typename T>
		T* get_component();

		template<typename T>
		T* get_component(std::string const& name);

		void set_position(XMFLOAT2 pos);

		void set_position(float x, float y);

		void set_rotation(float angle);
		void set_rotation(XMVECTOR quat);

		void attach_to(Entity* parent) 
		{ 
			_parent = parent; 
		}

		XMFLOAT3 get_local_position() const;
		XMFLOAT3 get_world_position() const;
		XMMATRIX get_local_transform() const;
		XMMATRIX get_world_transform() const;

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

		XMFLOAT3 _pos;
		XMVECTOR _rot;

		std::vector<Component*> m_Components;

	};

	template<typename T, typename...Args>
	T* Entity::create_component(std::string const& name = "Unnamed", Args...args)
	{
		T* comp = new T(name, args...);
		m_Components.push_back(comp);

		comp->on_attach(this);

		return comp;
	}

	template<typename T>
	T* Entity::get_component(std::string const& name)
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), [](auto const& comp) {
			return comp->name.compare(name);
		});

		if (it != m_Components.end())
			return *it;
		return nullptr;
	}

	template<typename T>
	T* Entity::get_component()
	{
		rtti::TypeInfo* info = rtti::Registry::template get<T>();
		return static_cast<T*>(get_component(info));
	}




}
