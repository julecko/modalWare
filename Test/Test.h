#ifndef TEST_H
#define TEST_H

#ifdef MYLIBRARY_EXPORTS
#define EXPORT_FUNCTION extern "C" __declspec(dllexport)
#else
#define EXPORT_FUNCTION
#endif

EXPORT_FUNCTION void hello();
EXPORT_FUNCTION int add(int a, int b);
EXPORT_FUNCTION void initialize()

#endif
