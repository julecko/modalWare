#include <iostream>
#include <Windows.h>
#include <thread>

typedef uint32_t (*SendMessageFunc)(const char*);
typedef const char* (*GetMessageFunc)();

int main() {
    // Load the DLL
    HMODULE hModule = LoadLibraryA("discord_bot.dll");
    if (!hModule) {
        std::cerr << "Could not load the DLL!" << std::endl;
        return 1;
    }

    SendMessageFunc send_message = (SendMessageFunc)GetProcAddress(hModule, "send_message");
    if (!send_message) {
        std::cerr << "Could not find send_message function in the DLL!" << std::endl;
        FreeLibrary(hModule);
        return 1;
    }

    GetMessageFunc get_message = (GetMessageFunc)GetProcAddress(hModule, "get_message");
    if (!get_message) {
        std::cerr << "Could not find get_message function in the DLL!" << std::endl;
        FreeLibrary(hModule);
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "TEST" << std::endl;
    const char* testMessage = "Hello from C++!";
    try {
        int result = send_message(testMessage);
        if (result == 0) {
            std::cout << "Message sent successfully!" << std::endl;
        } else {
            std::cerr << "Failed to send message! Result code: " << result << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred while sending message: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "An unknown error occurred while sending message." << std::endl;
    }
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