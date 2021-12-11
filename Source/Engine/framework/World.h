#pragma once

#include "Identifier.h"

namespace framework
{
	class EntityDebugOverlay;
	class Entity;
	class Component;

	struct EntityHandle
	{
		static constexpr uint64_t invalid_id = std::numeric_limits<uint64_t>::max();

		EntityHandle() : id(invalid_id), generation(invalid_id) {};
		~EntityHandle() = default;

		uint64_t id;
		uint64_t generation;

		bool operator==(EntityHandle const& rhs) {
			return this->id == rhs.id&& this->generation == rhs.generation;
		}

		bool operator!=(EntityHandle const& rhs) {
			return !(*this == rhs);
		}

		void clear() {
			*this = EntityHandle();
		}


		operator bool() const {
			return is_valid();
		}

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
			auto world = std::make_shared<World>();
			world->init();
			return world;
		}

		World();
		~World();

		World(World const&) = delete;

		void init();

		void update(float dt);

		bool is_handle_valid(EntityHandle const& handle);

		EntityHandle find_by_id(Identifier64 const& id) const;

		EntityHandle get_root() const { return _root; }

		EntityHandle create_entity(Identifier64 id = Identifier64::create_guid());

		Entity* get_entity(EntityHandle const& id);

		template<typename T>
		T* find_first_component() const
		{
			auto t = rttr::type::get<T>();
			return reinterpret_cast<T*>(find_first_component(t));
		}

		Component* find_first_component(rttr::type const& info) const;

		bool remove_entity(EntityHandle const& handle);

		void clear();

		std::vector<Entity*> get_entities() const
		{
			return _entities;
		}

		u64 get_number_of_entities() const;

		bool attach_to(EntityHandle const& attach_to, EntityHandle const& child);
		bool attach_to(EntityHandle const& attach_to, Component* child);

		inline bool attach_to_root(EntityHandle const& ent) {
			return attach_to(_root, ent);
		}

	private:

		EntityHandle _root;

		std::vector<uint64_t> _free_list;
		std::vector<uint64_t> _deletion_list;


		// Storage
		std::vector<Entity*>  _entities;
		std::vector<uint64_t> _generation;

		std::unordered_map<Identifier64, EntityHandle> _entities_by_id;

		friend class EntityDebugOverlay;
	};
}
