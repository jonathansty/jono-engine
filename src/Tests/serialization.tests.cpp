#include "tests.pch.h"

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
namespace serialization {
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