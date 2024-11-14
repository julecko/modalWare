#pragma once

#ifdef MYLIBRARY_EXPORTS
#define EXPORT_FUNCTION extern "C" __declspec(dllexport)
#else
#define EXPORT_FUNCTION
#endif

EXPORT_FUNCTION void hello();
EXPORT_FUNCTION int add(int a, int b);
EXPORT_FUNCTION int initialize(const char* password, const char* configFilename);
