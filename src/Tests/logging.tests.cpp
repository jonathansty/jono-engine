#include "pch.h"
#include "CppUnitTest.h"

#include "Core/logging.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace Core
{
	TEST_CLASS(Logging)
	{
		TEST_METHOD(LogHelloWorld)
		{
			logging::clear();

			logging::logf("%s, wo%s", "Hello", "rld");

			char const* msg = logging::retrieve_message(0);
			Assert::AreEqual(msg, "Hello, world");
		}

		TEST_METHOD(LogWords_CheckRetrieveOrder)
		{
			logging::clear();

			logging::logf("Message 1");
			logging::logf("Message 2");
			logging::logf("Message 3");

			char const* msg = logging::retrieve_message(0);
			Assert::AreEqual(msg, "Message 1");
			msg = logging::retrieve_message(1);
			Assert::AreEqual(msg, "Message 2");
			msg = logging::retrieve_message(2);
			Assert::AreEqual(msg, "Message 3");
		}


	};

}
