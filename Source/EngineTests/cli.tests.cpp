#include "CppUnitTest.h"

#include "CommandLine.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EngineTests {
	TEST_CLASS(CoreTests) {

	public:
		TEST_METHOD(cli_parse) {
			std::string msg = "-Flag1 -Flag2 -NumberValue=123 -BoolValue=false -BoolValue2=true -StringValue=Test";
			cli::CommandLine cmd = cli::parse(msg);
			cli::CommandLine expected = {
				"-Flag1", "-Flag2", "-NumberValue123", "-BoolValue=false", "-BoolValue2=true", "-StringValue=Test"
			};
			Assert::AreEqual(cmd[0], std::string("-Flag1"));
			Assert::AreEqual(cmd[1], std::string("-Flag2"));
			Assert::AreEqual(cmd[3], std::string("-BoolValue=false"));
			Assert::AreEqual(cmd[5], std::string("-StringValue=Test"));
		}
	};

}