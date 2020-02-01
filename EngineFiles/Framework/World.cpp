#include "stdafx.h"
#include "rtti/rtti.h"
#include "World.h"

#include "Entity.h"

using namespace framework;

World::World()
	: _entities()
	, _root(new Entity())
{
	_entities.reserve(1000);
	_root->set_name("Root");
}

World::~World()
{
	for (Entity* it : _entities)
	{
		delete it;
	}

	_entities.clear();

	delete _root;

}

void World::update(float dt)
{
	for (EntityId const& id : _deletion_list)
	{
		delete _entities[id];
		_entities[id] = nullptr;
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

void World::render()
{
	for (auto it : _entities)
	{
		if (it)
		{
			it->render();
		}
	}
}
