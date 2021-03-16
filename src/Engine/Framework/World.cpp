#include "engine.stdafx.h"
#include "World.h"

#include "Entity.h"
#include "Component.h"

using namespace framework;

EntityHandle::EntityHandle(uint64_t id, uint64_t generation, std::weak_ptr<World> world)
	: id(id)
	, generation(generation)
	, world(world)
{
}

bool EntityHandle::is_valid() const
{
	if (world.expired())
		return false;

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
: _root()
{

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
}

void World::update(float dt)
{
	for (uint64_t const& id : _deletion_list)
	{
		// Free the entity
		delete _entities[id];
		_entities[id] = nullptr;

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
		if(_entities[i]) {
			if (auto comp = _entities[i]->get_component(info); comp)
			{
				return comp;
			}
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

	++_generation[handle.id];

	return true;
}

bool framework::World::attach_to(EntityHandle const& attach_to, EntityHandle const& child) {

	auto current_parent = _entities[child.id]->_parent;
	if (current_parent) {
		current_parent->_children.erase(std::find(current_parent->_children.begin(), current_parent->_children.end(), child));
	}

	_entities[child.id]->_parent = attach_to;

	// update parent
	_entities[attach_to.id]->_children.push_back(child);

	return true;
}

bool framework::World::attach_to(EntityHandle const& attach_to, Component* child) {

	if(child->_parent) {
		child->on_detach(child->_parent);
	}

	_entities[attach_to.id]->add_component(child);

	// attach to new parent
	child->_parent = _entities[attach_to.id];
	child->on_attach(_entities[attach_to.id]);

	return true;
}

void framework::World::init() {

	// Create the root entity 
	Entity* ent = new Entity();

	std::size_t id = _entities.size();

	// If we have free slots
	if (!_free_list.empty()) {
		id = _free_list[_free_list.size() - 1];
		_free_list.pop_back();
	}
	// No free slots means we need to create some
	else {
		_entities.push_back(ent);
	}

	// Update our generation it's size
	if (_generation.size() <= id) {
		//TODO: Abstract vector resizing to be more robust
		_generation.resize(id + 1000, 0);
	}

	auto w = shared_from_this();
	auto handle = EntityHandle(id, _generation[id], w);
	_root = handle;
	_root->set_name("Root");
}

void framework::World::clear() {

	// Clear all the children of our root
	_root->_children.clear();

	// process each entity except our root
	for (uint64_t id = 1; id < _entities.size(); ++id) {

		// Free the entity
		delete _entities[id];
		_entities[id] = nullptr;

		// Invalidate the generation for all entities
		++_generation[id];
	}

	_entities.resize(1);
}
