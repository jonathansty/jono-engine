#pragma once
#include "rtti/rtti.h"
namespace framework
{
	class Entity;

	class Component
	{
		REFLECT(Component)
	public:
		Component() {};
		Component(std::string const& name);

		virtual ~Component();

		virtual void on_attach(Entity* _ent) { _parent = _ent; }
		virtual void on_detach(Entity* _ent) { _parent = nullptr; }

		virtual void update(float dt) {}
		virtual void render() {}

		Entity* get_entity() const 
		{
			return _parent;
		}


	private:
		Entity* _parent;
		std::string _name;
	};

	class Entity
	{
	public:
		Entity(XMFLOAT2 pos);
		virtual ~Entity();

		virtual void update(float dt) 
		{
			for (auto& el : m_Components)
			{
				el->update(dt);
			}
		}

		virtual void render()
		{
			for (auto& el : m_Components)
			{
				el->render();
			}
		}

		template<typename T, typename...Args>
		T* create_component(std::string const& name, Args...args)
		{
			T* comp = new T(name, args...);
			m_Components.push_back(comp);

			comp->on_attach(this);

			return comp;
		}

		template<typename T>
		T* get_component(std::string const& name)
		{
			auto it =std::find(m_Components.begin(), m_Components.end(), [](auto const& comp) {
				return comp->name.compare(name);
			});

			if (it != m_Components.end())
				return *it;
			return nullptr;
		}

		template<typename T>
		T* get_component()
		{
			rtti::TypeInfo* info = rtti::Registry::template get<T>();
			for (Component* c : m_Components)
			{
				if (c->get_type()->is(info))
					return (T*)c;
			}
			return nullptr;
		}




		void set_position(XMFLOAT2 pos)
		{
			_pos = pos;
		}

		void set_position(float x, float y)
		{
			set_position({ x,y });
		}

		XMFLOAT2 get_position() const
		{
			return _pos;
		}

	protected:
		XMFLOAT2 _pos;

		std::vector<Component*> m_Components;

	};

	class World final
	{
	public:
		using EntityId = std::size_t;

		World();

		~World();

		World(World const&) = delete;

		void update(float dt);

		void render();

		template<typename T = Entity, typename...Args>
		EntityId create_entity(Args... args)
		{
			T* obj = new T(args...);
			
			std::size_t id = _entities.size();
			_entities.push_back(obj);

			return id;
		}

		Entity* get_entity(EntityId id)
		{
			assert(id >= 0 && id < _entities.size());
			return _entities[id];
		}
		bool remove_entity(EntityId id)
		{
			auto it = std::find(_deletion_list.begin(), _deletion_list.end(), id);
			assert(it == _deletion_list.end());
			_deletion_list.push_back(id);
		}

	private:
		std::vector<size_t> _free_list;
		std::vector<size_t> _deletion_list;

		std::vector<Entity*> _entities;

	};
}
