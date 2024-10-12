#ifndef TEST_H
#define TEST_H

#ifdef MYLIBRARY_EXPORTS
#define MYLIBRARY_API __declspec(dllexport)
#else
#define MYLIBRARY_API
#endif

extern "C" {
    MYLIBRARY_API void hello();
    MYLIBRARY_API int add(int a, int b);
}

#endif
