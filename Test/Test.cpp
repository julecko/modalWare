// mylibrary.cpp
#include <iostream>

#ifdef MYLIBRARY_EXPORTS
#define MYLIBRARY_API __declspec(dllexport)
#else
#define MYLIBRARY_API
#endif

extern "C" {
    MYLIBRARY_API void hello() {
        std::cout << "Hello from the DLL!" << std::endl;
    }

    MYLIBRARY_API int add(int a, int b) {
        return a + b;
    }
}
