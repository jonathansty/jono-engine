#pragma once
#include "rtti/rtti.h"

class EntityDebugOverlay;

namespace framework
{
	class Entity;
	class Component;

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
			obj->attach_to(_root);

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

		std::vector<Entity*> get_entities() const
		{
			return _entities;
		}

	private:
		std::vector<size_t> _free_list;
		std::vector<size_t> _deletion_list;

		Entity* _root;
		std::vector<Entity*> _entities;
		friend EntityDebugOverlay;
	};
}
