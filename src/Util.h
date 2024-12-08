#pragma once
#include "FunctionPointer.h"
#include "Types.h"

enum class FindingResultID : uint8_t {
    SUCCESS = 0,
    NO_MODULE = 1,
    NO_FUNCTION = 2
};
struct FindingResult {
    FindingResultID result_id;
    FunctionPointer fp;
    FindingResult(FindingResultID id = FindingResultID::NO_MODULE)
        : result_id(id) {}
};
struct ConvertionResult {
    StatusCode status_code;
    std::any value;
};
//Formatting
namespace format {
    std::string functionTypeToString(FunctionType functionType);
    std::string valueTypeToString(ValueType type);
    std::string callingTypeToString(CallingType callingType);
    ConvertionResult stringToAny(const std::string& value, const ValueType& type);
    std::string getPrintableModules(const std::unordered_map<std::string, ModuleStruct>& modules, const bool& pretty = false);
    std::string getPrintableFunctions(const std::unordered_map<std::string, FunctionPointer>& functions, const int& tabs, const bool& pretty = false);
}
FindingResult findFunction(const std::unordered_map<std::string, ModuleStruct>& modules, std::string moduleName, std::string functionName);