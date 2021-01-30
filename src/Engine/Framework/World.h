#pragma once
#include "rtti/rtti.h"

class EntityDebugOverlay;

namespace framework
{
	class Entity;
	class Component;

	struct EntityHandle
	{
		EntityHandle(){};
		~EntityHandle() = default;

		uint64_t id;
		uint64_t generation;


		bool is_valid() const;

		Entity* operator->() const;
		Entity* get() const;

		operator Entity*()
		{
			return get();
		}

	private:
		EntityHandle(uint64_t id, uint64_t generation, std::weak_ptr<class World> world);

		// Non-Owning pointer to the world
		std::weak_ptr<class World> world;

		friend class World;
	};

	class World final : public std::enable_shared_from_this<World>
	{
	public:
		using EntityId = EntityHandle;

		static std::shared_ptr<World> create() {
			return std::make_shared<World>();
		}

		World();
		~World();

		World(World const&) = delete;

		void update(float dt);

		bool is_handle_valid(EntityHandle const& handle);

		template<typename T = Entity, typename...Args>
		EntityHandle create_entity(Args ... args);

		Entity* get_entity(EntityHandle const& id);

		template<typename T>
		T* find_first_component() const
		{
			return reinterpret_cast<T*>(find_first_component(rtti::Registry::get<T>()));
		}

		Component* find_first_component(rtti::TypeInfo const* info) const;

		bool remove_entity(EntityHandle const& handle);

		std::vector<Entity*> get_entities() const
		{
			return _entities;
		}

	private:

		Entity* _root;

		std::vector<uint64_t> _free_list;
		std::vector<uint64_t> _deletion_list;

		// Storage
		std::vector<Entity*>  _entities;
		std::vector<uint64_t> _generation;

		friend EntityDebugOverlay;
	};

	template <typename T, typename ... Args>
	EntityHandle World::create_entity(Args ... args)
	{
		T* obj = new T(args...);

		std::size_t id = _entities.size();

		// If we have free slots
		if(!_free_list.empty())
		{
			id = _free_list[_free_list.size() - 1];
			_free_list.pop_back();
		}
		// No free slots means we need to create some
		else
		{
			_entities.push_back(obj);
		}


		// Update our generation it's size
		if(_generation.size() <= id)
		{
			//TODO: Abstract vector resizing to be more robust
			_generation.resize(id + 1000, 0);
		}

		obj->attach_to(_root);

		auto w = shared_from_this();
		return EntityHandle(id, _generation[id], w);
	}
}
