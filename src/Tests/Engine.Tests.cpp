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

	public:
		int _a;
		int _b;
	};

	class MockTypeB : public MockTypeA
	{
		REFLECT(MockTypeB);

	public:
		void add(double d) {
			_c += d;
		}

		double _c;
	};

	IMPL_REFLECT(MockTypeA)
	{
		type.register_property("a", &MockTypeA::_a);
		type.register_property("b", &MockTypeA::_b);
	}

	IMPL_REFLECT(MockTypeB)
	{
		type.bind_parent<MockTypeA>();
		type.register_property("c", &MockTypeB::_c);

		type.register_property<MockTypeB, double>("custom_c", 
			[](MockTypeB* obj, double const* val) 
			{
				obj->_c = (*val) * 20.0; 
			}, 
			[](MockTypeB* obj, double** val) 
			{
				*val = &obj->_c;
			}
		);
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

		TEST_METHOD(MockTypeB_Inherits_MockTypeA)
		{
			rtti::TypeInfo* info = rtti::Registry::get<MockTypeB>();
			Assert::IsTrue(info->inherits(rtti::Registry::get<MockTypeA>()));
		}

		TEST_METHOD(MockTypeB_HasInheritedProperties)
		{
			rtti::TypeInfo* info = rtti::Registry::get<MockTypeB>();
			Assert::IsNotNull(info->find_property("a"));
			Assert::IsNotNull(info->find_property("b"));
			Assert::IsNotNull(info->find_property("c"));
			Assert::IsNotNull(info->find_property("custom_c"));
		}

		TEST_METHOD(MockTypeB_SetUsingCustomProperty)
		{
			MockTypeB obj{};
			rtti::Object dynamic_obj = rtti::Object::create_as_ref(&obj);

			rtti::TypeInfo* info = rtti::Registry::get<MockTypeB>();
			dynamic_obj.set_property("custom_c", -1.0);
			double value = dynamic_obj.get_property<double>("custom_c");
			Assert::AreEqual(value, -20.0, 0.01);
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
