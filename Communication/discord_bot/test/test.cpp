#include <iostream>
#include <Windows.h>
#include <thread>

typedef const char* (*GetMessageFunc)();

int main() {
    // Load the DLL
    HMODULE hModule = LoadLibraryA("discord_bot.dll");
    if (!hModule) {
        std::cerr << "Could not load the DLL!" << std::endl;
        return 1;
    }

    // Get the get_message function pointer
    GetMessageFunc get_message = (GetMessageFunc)GetProcAddress(hModule, "get_message");
    if (!get_message) {
        std::cerr << "Could not find get_message function in the DLL!" << std::endl;
        FreeLibrary(hModule);
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Retrieve messages in a loop
    for (int i = 0; i < 5; ++i) {  // Retrieve up to 5 messages
        const char* message = get_message();
        if (message) {
            std::cout << "Received message: " << message << std::endl;
        } else {
            std::cout << "No new messages in queue." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    FreeLibrary(hModule);
    return 0;
}