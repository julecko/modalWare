#include <iostream>
#include <Windows.h>

typedef int (*DllMainFunc)(HINSTANCE, DWORD, LPVOID);
typedef int (*InitializeFunc)();

int main() {
    HMODULE hModule = LoadLibraryA("discord_bot.dll");
    if (!hModule) {
        std::cerr << "Could not load the DLL!" << std::endl;
        return 1;
    }

    InitializeFunc dllMain = (InitializeFunc)GetProcAddress(hModule, "Initialize");
    if (dllMain) {
        int result = dllMain();
        std::cout << "OnProcessAttach returned: " << result << std::endl;

        result = dllMain();
        std::cout << "OnProcessDetach returned: " << result << std::endl;
    }

    FreeLibrary(hModule);
    return 0;
}