// mylibrary.cpp
#include <iostream>
#include <windows.h>
#include "../ModulConfig/Config.h"

#ifdef MYLIBRARY_EXPORTS
#define EXPORT_FUNCTION extern "C" __declspec(dllexport)
#else
#define EXPORT_FUNCTION
#endif

EXPORT_FUNCTION void hello() {
    std::cout << "Hello from the DLL!" << std::endl;
}
EXPORT_FUNCTION int add(int a, int b) {
    return a + b;
}
EXPORT_FUNCTION void initialize(const char* configFilename) {
    std::string path(configFilename);
    ConfigManager manager(path);

    manager.addValue("VERSION", "1.0");
    manager.addValue("TEST_VALUE", "SOMETHING");
    std::cout << "Config configured" << std::endl;
}