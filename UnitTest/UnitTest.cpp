#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "CppUnitTest.h"
#include "../Test/Test.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Modules
{   
    typedef int (*pInitializeFunc)(const char*, const char*);
    TEST_CLASS(TestLibraryTesting)
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
            if (std::filesystem::exists(configFilename))
            {
                std::ifstream file(configFilename);
                std::string fileContents((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

                Assert::IsTrue(fileContents.find("VERSION") != std::string::npos, L"VERSION not found in file");
                Assert::IsTrue(fileContents.find("TEST_VALUE") != std::string::npos, L"TEST_VALUE not found in file");
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
        TEST_METHOD(TestInitialize)
        {
            HMODULE hModule = LoadLibraryA("discord_bot.dll");
            Assert::IsNotNull(hModule, L"Failed to load DLL");

            pInitializeFunc initialize = (pInitializeFunc)GetProcAddress(hModule, "initialize");
            Assert::IsNotNull((void*)initialize, L"Failed to find 'initialize' function");

            int result = initialize("12234", "discord_config.conf");
            Assert::IsTrue(result == 0 || result == 1, L"Initialization of discord bot did not finish successfully");

            FreeLibrary(hModule);
        }
    };
}