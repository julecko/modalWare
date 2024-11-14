#include <iostream>
#include <atomic>
#include "../ConfigManager/Config.h"

#ifdef MYLIBRARY_EXPORTS
#define EXPORT_FUNCTION extern "C" __declspec(dllexport)
#else
#define EXPORT_FUNCTION
#endif

std::atomic<bool> initialized{ false };

EXPORT_FUNCTION void hello() {
    if (!initialized.load()) {
        return;
    }
    std::cout << "Hello from the library!" << std::endl;
}
EXPORT_FUNCTION int add(int a, int b) {
    if (!initialized.load()) {
        return 0;
    }
    return a + b;
}
EXPORT_FUNCTION int initialize(const char* password, const char* configFilename) {
    bool expected = false;
    if (std::string(password) != "12234") {
        std::cout << "Password did not match, cancelling: " << password << std::endl;
        return -1;
    }
    if (!initialized.compare_exchange_strong(expected, true)) {
        std::cerr << "Warning: Library has already been initialized." << std::endl;
        return 1;
    }
    if (std::filesystem::exists(configFilename)) {
        std::cout << "Configuration file already exists, initialization skipped." << std::endl;
        return 1;
    }
    std::string path(configFilename);
    ConfigManager manager(path);

    manager.addValue("VERSION", "1.0");
    manager.addValue("TEST_VALUE", "SOMETHING");
    return 0;
}
