#pragma once
#include <string>
#include <windows.h>

#include "./FunctionPointer.h"

struct ModuleStruct {
    std::string version;
    HMODULE hmodule;
    std::unordered_map<std::string, FunctionPointer> functions;
};

std::unordered_map<std::string, ModuleStruct> getExtensions();