// mylibrary.cpp
#include <iostream>
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
