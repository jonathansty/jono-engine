#include "pch.h"
#include "CppUnitTest.h"

#include "Framework/World.h"
#include "Framework/Entity.h"
#include "Serialization.h"
#include "core/PlatformIO.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace FrameworkTests
{
	using namespace framework;

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

	struct MockA {
		MockA() = default;
		MockA(MockA const& rhs)
				: a(rhs.a), d(rhs.d) 
		{}

		int a;
		float d;
	};

	struct MockB {
		MockB() = default;
		MockB(MockB const& rhs) : value(rhs.value), data(data) {

		}

		MockA value;
		std::string data;
	};
	RTTR_REGISTRATION{
		using namespace rttr;

		registration::class_<MockA>("MockA")
			.constructor<>()()
			.property("a", &MockA::a)
			.property("d", &MockA::d);
		
		registration::class_<MockB>("MockB")
			.constructor<>()()
			.property("value", &MockB::value)
			.property("data", &MockB::data);

	}

	TEST_CLASS(SerializationBinaryTests) {
		TEST_METHOD(serialize_simple) {

				rttr::type t = rttr::type::get<MockA>().create().get_type();

				MockB data{};
				data.value.a = 100.0f;
				data.value.d = -75.0f;
				data.data = "Test123@";

				char path[512];
				GetCurrentDirectoryA(512, path);

				auto io = IO::create();
				io->mount("./");
				if (auto file = io->open("tests.bin", IO::Mode::Write, true); file) {
					serialization::serialize_instance<IO::Mode::Write>(file, data);
				}

				if (auto file = io->open("tests.bin", IO::Mode::Read, true); file) {

					u64 hash = serialization::read<u64>(file);
					file->seek(0, IO::SeekMode::FromBeginning);

					rttr::type t = helpers::get_type_by_id(hash);
					rttr::variant obj = t.create();


					bool result = serialization::serialize_instance<IO::Mode::Read>(file, obj);

					std::shared_ptr<MockB> read_data = obj.get_value<std::shared_ptr<MockB>>();
					Assert::AreEqual(data.value.a, read_data->value.a);
					Assert::AreEqual(data.value.d, read_data->value.d);
					Assert::AreEqual(data.data, read_data->data);
				
				}
		}
	};

}
