#include "tests.pch.h"

using namespace framework;

namespace framework {

TEST_CLASS(WorldTests)
{
public:

	TEST_METHOD(world_create)
	{
		auto world = World::create();
		Assert::IsTrue(world->get_root().is_valid());

		auto ent = world->get_entities();
		Assert::AreEqual<size_t>(ent.size(), 1, L"An empty world expects 1 entity (root)!");
	}

	TEST_METHOD(world_create_entity)
	{
		auto world = World::create();

		framework::EntityHandle entity = world->create_entity();
		Assert::IsTrue(entity.is_valid() && world->get_entities().size() == 2);
	}

	TEST_METHOD(world_entities_add_remove)
	{
		auto world = framework::World::create();

		framework::EntityHandle ent0 = world->create_entity();
		framework::EntityHandle ent1 = world->create_entity();
		framework::EntityHandle ent2 = world->create_entity();

		world->remove_entity(ent1);

		world->update(0.33f);

		auto ent3 = world->create_entity();
		Assert::AreEqual<uint64_t>(2, ent3.id, L"Unexpected Entity ID!");
		Assert::AreEqual<uint64_t>(1, ent3.generation, L"Unexpected generation!");
	}

	TEST_METHOD(world_clear) {
		auto world = World::create();

		std::vector<EntityHandle> handles;
		for (int i = 0; i < 50; ++i) {
			handles.push_back(world->create_entity());
		}

		world->clear();

		for (int i = 0; i < 50; ++i) {
			Assert::IsFalse(handles[i].is_valid());

			EntityHandle n = world->create_entity();
			Assert::IsTrue(n.is_valid());
		}

	}
};

TEST_CLASS(EntityHandleTests) {

	TEST_METHOD(handle_create_invalid) {
		EntityHandle handle{};
		Assert::IsFalse(handle.is_valid());
	}

	TEST_METHOD(handle_create_valid) {
		auto w = World::create();

		EntityHandle handle = w->create_entity();
		Assert::IsTrue(handle.is_valid());
		Assert::IsTrue(handle.id == 1);

		handle = w->create_entity();
		Assert::IsTrue(handle.is_valid());
		Assert::IsTrue(handle.id == 2);
	}

	TEST_METHOD(handle_invalid_after_destroy) {
		auto w = World::create();
		EntityHandle handle = w->create_entity();

		Assert::IsTrue(w->remove_entity(handle));

		Assert::IsFalse(handle.is_valid());

	}
};

}
