#include "CppUnitTest.h"
#include "../Test/Test.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(MyLibraryTests)
    {
    public:
        const char* correctPassword = "12234";
        const char* configFilename = "test_config.json";

        TEST_METHOD(TestInitialize)
        {
            const char* incorrectPassword = "wrongPassword";

            int result = initialize(correctPassword, configFilename);
            Assert::IsTrue(result == 0 || result == 1, L"result is neither 0 nor 1");

            result = initialize(incorrectPassword, configFilename);
            Assert::AreEqual(-1, result);
        }
        TEST_METHOD(TestAdd)
        {
            int result1 = initialize(correctPassword, configFilename);
            int result = add(3, 5);
            Assert::AreEqual(8, result);
        }
    };
}