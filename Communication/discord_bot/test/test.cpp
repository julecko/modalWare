#include <Windows.h>
#include <iostream>
#include <thread>
#include <atomic>

typedef void(*AttachProcessFunc)();
typedef bool (*VerifyCode)(const char*);

int main() {
    // Load the DLL
    HMODULE hModule = LoadLibrary(TEXT("discord_bot.dll")); // Change this to your DLL path
    if (!hModule) {
        std::cerr << "Failed to load the DLL!" << std::endl;
        return 1;
    }

    // Get function pointers
    AttachProcessFunc attach_process = (AttachProcessFunc)GetProcAddress(hModule, "attach_process");
    if (!attach_process) {
        std::cerr << "Failed to get function addresses!" << std::endl;
        FreeLibrary(hModule);
        return 1;
    }
    VerifyCode verify_code = (VerifyCode)GetProcAddress(hModule, "verify");
        if (verify_code) {
            const char* code = "12234";
            if (verify_code(code)) {
                std::cout << "Verification succeeded!" << std::endl;
            } else {
                std::cout << "Verification failed!" << std::endl;
            }
        } else {
            std::cerr << "Could not find verify function!" << std::endl;
        }

    std::thread bot_thread(attach_process);
    bot_thread.detach();
    std::cout << "Discord bot started." << std::endl;

    // Wait for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Discord bot stopped." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    FreeLibrary(hModule);
    return 0;
}