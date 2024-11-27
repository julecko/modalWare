#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "CppUnitTest.h"
#include "../Test/Test.h"
#include "../src/FunctionPointer.h"
#include "../src/ModuleLoading.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Modules
{   
    typedef int (*pInitializeFunc)(const char*, const char*);
    const char* correctPassword = "12234";
    TEST_CLASS(TestLibraryTesting)
    {
    public:
        const char* configFilename = "test.conf";

        TEST_METHOD(TestInitialize)
        {
            const char* incorrectPassword = "wrongPassword";

            int result = initialize(correctPassword, configFilename);
            Assert::IsTrue(result == 0 || result == 1, L"result is neither 0 nor 1");

            result = initialize(incorrectPassword, configFilename);
            Assert::AreEqual(-1, result);
            if (std::filesystem::exists(configFilename))
            {
                std::ifstream file(configFilename);
                std::string fileContents((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

                Assert::IsTrue(fileContents.find("version") != std::string::npos, L"VERSION not found in file");
                Assert::IsTrue(fileContents.find("test_value") != std::string::npos, L"TEST_VALUE not found in file");
            }
        }
        TEST_METHOD(TestAdd)
        {
            int result1 = initialize(correctPassword, configFilename);
            int result = add(3, 5);
            Assert::AreEqual(8, result);
        }

    };
    TEST_CLASS(DiscordCommunicationLibraryTesting)
    {
    public:
        typedef int (*pInitializeFunc)(const char*, const char*);
        TEST_METHOD(TestInitialize)
        {
            HMODULE hModule = LoadLibraryA("discord_bot.dll");
            Assert::IsNotNull(hModule, L"Failed to load DLL");

            pInitializeFunc initialize = (pInitializeFunc)GetProcAddress(hModule, "initialize");
            Assert::IsNotNull((void*)initialize, L"Failed to find 'initialize' function");

            int result = initialize(correctPassword, "discord_config.conf");
            Assert::IsTrue(result == 0 || result == 1, L"Initialization of discord bot did not finish successfully");

            FreeLibrary(hModule);
        }
    };
    TEST_CLASS(FunctionPointerTesting)
    {
    public:
        typedef int (*pAddFunc)(const int, const int);
        TEST_METHOD(TestCall)
        {
            HMODULE hModule = LoadLibraryA("test.dll");
            Assert::IsNotNull(hModule, L"Failed to load DLL");

            pInitializeFunc initialize = (pInitializeFunc)GetProcAddress(hModule, "initialize");
            Assert::IsNotNull((void*)initialize, L"Failed to find 'initialize' function");

            int result = initialize(correctPassword, "test.conf");
            Assert::IsTrue(result == 0 || result == 1, L"Initialization of testing dll did not finish successfully");

            FunctionPointer fp;
            
            fp.set("add", hModule);
            result = fp.callSingle<int, int, int>(5, 6);
            Assert::AreEqual(result, 11);

            fp.function_type = FunctionType::SINGLE;
            result = fp.manualCall<int, int, int>(5, 6);
            Assert::AreEqual(result, 11);

            fp.return_type = ValueType::INT_TYPE;
            fp.arg1_type = ValueType::INT_TYPE;
            fp.arg2_type = ValueType::INT_TYPE;
            fp.argCount = 2;
            FunctionResult new_result = fp.autoCall(5, 6);
            Assert::AreEqual(11, new_result.value.int_result);
        }
    };
    TEST_CLASS(ModuleLoadingTesting)
    {
        TEST_METHOD(ConfigurationLoadingTest)
        {
            int result = initialize(correctPassword, "test.conf");
            Assert::IsTrue(result == 0 || result == 1, L"Initialization of testing dll did not finish successfully");

            std::filesystem::path filePath("test.conf");
            //loadFunctionsConfig(filePath);
        }
    };
}