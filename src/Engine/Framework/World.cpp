#include "engine.stdafx.h"
#include "World.h"

#include "Entity.h"

using namespace framework;

EntityHandle::EntityHandle(uint64_t id, uint64_t generation, std::weak_ptr<World> world)
	: id(id)
	, generation(generation)
	, world(world)
{
}

bool EntityHandle::is_valid() const
{
	auto p_world = world.lock();
	return p_world->is_handle_valid(*this);
}

Entity* EntityHandle::operator->() const
{
	return get();
}

Entity* EntityHandle::get() const
{
	if (is_valid())
	{
		auto p_world = world.lock();
		return p_world->get_entity(*this);
	}
	return nullptr;
}

World::World()
	: _root(new Entity())
{
	_root->set_name("Root");

	_entities.reserve(1000);
	_generation.reserve(1000);
}

World::~World()
{
	for (Entity* it : _entities)
	{
		delete it;
	}

	_entities.clear();
	_generation.clear();

	delete _root;
	_root = nullptr;
}

void World::update(float dt)
{
	for (uint64_t const& id : _deletion_list)
	{
		// Free the entity
		delete _entities[id];
		_entities[id] = nullptr;

		// Invalidate the generation
		++_generation[id];

		// Add this slot to the free list
		_free_list.push_back(id);
	}

	for (auto it : _entities)
	{
		if (it)
		{
			it->update(dt);
		}
	}
}

bool World::is_handle_valid(EntityHandle const& handle)
{
	return _generation[handle.id] == handle.generation;
}

Entity* World::get_entity(EntityHandle const& id)
{
	assert(id.id >= 0 && id.id < _entities.size());
	if (id.generation != _generation[id.id])
		return nullptr;

	return _entities[id.id];
}

Component* World::find_first_component(rttr::type const& info) const
{
	for (std::size_t i = 0; i < _entities.size(); ++i)
	{
		if (auto comp = _entities[i]->get_component(info); comp)
		{
			return comp;
		}
	}

	return nullptr;
}

bool World::remove_entity(EntityHandle const& handle)
{
	// Check if this id hasn't been queued yet for debugging
	// and then queue it up for deletion in the update loop
	auto it = std::find(_deletion_list.begin(), _deletion_list.end(), handle.id);
	assert(it == _deletion_list.end());
	_deletion_list.push_back(handle.id);

	return true;
}
