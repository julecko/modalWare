#pragma once
#include "Types.h"

//Formatting
namespace format {
    std::string functionTypeToString(FunctionType functionType);
    std::string valueTypeToString(ValueType type);
    std::string callingTypeToString(CallingType callingType);
    std::string getPrintableModules(const std::unordered_map<std::string, ModuleStruct>& modules, const bool& pretty = false);
    std::string getPrintableFunctions(const std::unordered_map<std::string, FunctionPointer>& functions, const int& tabs, const bool& pretty = false);
}