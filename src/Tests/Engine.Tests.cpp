#include "pch.h"
#include "CppUnitTest.h"

#include "Framework/World.h"
#include "Framework/Entity.h"
#include "rtti/rtti.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace Core
{
	class MockTypeA
	{
		REFLECT(MockTypeA);
	};

	class MockTypeB : public MockTypeA
	{
		REFLECT(MockTypeB);

	public:

	};

	IMPL_REFLECT(MockTypeA)
	{
	}

	IMPL_REFLECT(MockTypeB)
	{
		type.bind_parent<MockTypeA>();
	}

	TEST_CLASS(RTTITests)
	{
		TEST_METHOD(registry_retrieve_primitive_type)
		{
			rtti::TypeInfo* type = rtti::Registry::get<int>();
			Assert::IsNotNull(type);

			Assert::AreEqual(sizeof(int), type->get_size(), L"Type information has unexpected size!");
			Assert::AreEqual("int", type->get_name(), L"Type name is incorrect!");
		}

		TEST_METHOD(registry_retrieve_mock_type_a)
		{
			auto type = rtti::Registry::get<MockTypeA>();
			Assert::IsNotNull(type);
			Assert::AreEqual(sizeof(MockTypeA), type->get_size());
		}

		TEST_METHOD(registry_retrieve_mock_type_b)
		{
			auto type = rtti::Registry::get<MockTypeB>();
			Assert::AreEqual(sizeof(MockTypeB), type->get_size());
			Assert::IsTrue(type->inherits(MockTypeA::get_static_type()));
		}

		TEST_METHOD(MockTypeA_get_static_type)
		{
			auto type = MockTypeA::get_static_type();
			Assert::IsNotNull(type);
		}

		TEST_METHOD(MockTypeB_get_static_type)
		{
			auto type = MockTypeB::get_static_type();
			Assert::IsNotNull(type);
		}
	};
}

namespace GameFrameworkTests
{
	TEST_CLASS(WorldTests)
	{
	public:
		TEST_METHOD(create_empty_world)
		{
			auto world = framework::World::create();
			Assert::AreEqual(int(world->get_entities().size()), 0, L"An empty world expects zero entities!");
		}

		TEST_METHOD(create_world_and_entities)
		{
			auto world = framework::World::create();
			Assert::AreEqual(int(world->get_entities().size()), 0, L"An empty world expects zero entities!");

			framework::EntityHandle entity = world->create_entity();
			Assert::IsNotNull(entity.get());
		}

		TEST_METHOD(add_remove_entities)
		{
			auto world = framework::World::create();
			Assert::AreEqual(int(world->get_entities().size()), 0, L"An empty world expects zero entities!");

			framework::EntityHandle ent0 = world->create_entity();
			framework::EntityHandle ent1 = world->create_entity();
			framework::EntityHandle ent2 = world->create_entity();

			world->remove_entity(ent1);

			world->update(0.33f);

			auto ent3 = world->create_entity();
			Assert::AreEqual<uint64_t>(1, ent3.id, L"Unexpected Entity ID!");
			Assert::AreEqual<uint64_t>(1, ent3.generation, L"Unexpected generation!");
		}


	};


}
