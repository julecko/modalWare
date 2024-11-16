#pragma once
#include <string>
#include <windows.h>
#include <filesystem>
#include <unordered_map>

#include "./FunctionPointer.h"

struct ModuleStruct {
    HMODULE hmodule;
    std::unordered_map<std::string, FunctionPointer> functions;
    std::unordered_map<std::string, std::string> metadata;
};

std::unordered_map<std::string, ModuleStruct> getExtensions();