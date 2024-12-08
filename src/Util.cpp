#include <sstream>
#include "FunctionPointer.h"
#include "Types.h"
#include "Util.h"

#define RED_TEXT(text) "\033[31m" + text + "\033[0m"
#define BLUE_TEXT(text) "\033[34m" + text + "\033[0m"
#define GREEN_TEXT(text) "\033[32m" + text + "\033[0m"
#define PURPLE_TEXT(text) "\033[35m" + text + "\033[0m"
#define LIGHT_BLUE_TEXT(text) "\033[1;34m" + text + "\033[0m"

#define RED_TEXT_IF(text, condition) (condition ? RED_TEXT(text) : (text))
#define BLUE_TEXT_IF(text, condition) (condition ? BLUE_TEXT(text) : (text))
#define GREEN_TEXT_IF(text, condition) (condition ? GREEN_TEXT(text) : (text))
#define PURPLE_TEXT_IF(text, condition) (condition ? PURPLE_TEXT(text) : (text))
#define LIGHT_BLUE_TEXT_IF(text, condition) (condition ? LIGHT_BLUE_TEXT(text) : (text))

namespace format {
    std::string functionTypeToString(FunctionType functionType) {
        switch (functionType) {
        case FunctionType::DEFAULT: return "DEFAULT";
        case FunctionType::SINGLE: return "SINGLE";
        case FunctionType::THREAD: return "THREAD";
        default: return "Unknown";
        }
    }
    std::string valueTypeToString(ValueType type) {
        switch (type) {
        case ValueType::DEFAULT_TYPE: return "DEFAULT_TYPE";
        case ValueType::NONE_TYPE: return "NONE_TYPE";
        case ValueType::INT_TYPE: return "INT_TYPE";
        case ValueType::CHAR_TYPE: return "CHAR_TYPE";
        case ValueType::FLOAT_TYPE: return "FLOAT_TYPE";
        default: return "Unknown";
        }
    }
    std::string callingTypeToString(CallingType callingType) {
        switch (callingType) {
        case CallingType::DEFAULT: return "DEFAULT";
        case CallingType::MANUAL: return "MANUAL";
        case CallingType::STARTUP: return "STARTUP";
        default: return "Unknown";
        }
    }
    ConvertionResult stringToAny(const std::string &value, const ValueType &type) {
        ConvertionResult result;
        result.status_code = StatusCode::FAILURE;
        switch (type) {
        case ValueType::DEFAULT_TYPE:
            break;
        case ValueType::NONE_TYPE:
            break;
        case ValueType::INT_TYPE:
            try {
                result.value = std::stoi(value);
            }
            catch (const std::exception&) {
                break;
            }
            break;
        case ValueType::FLOAT_TYPE:
            try {
                result.value = std::stof(value);
            }
            catch (const std::exception&) {
                break;
            }
            break;
        case ValueType::CHAR_TYPE:
            result.value = value.c_str();
            break;
        }
        return result;
    }
    std::string getPrintableModules(const std::unordered_map<std::string, ModuleStruct>& modules, const bool& pretty) {
        std::stringstream outputString;
        for (const auto& [moduleName, moduleStruct] : modules) {
            outputString << "Module: " << GREEN_TEXT_IF(moduleName, pretty) << "\n";
            outputString << getPrintableFunctions(moduleStruct.functions, 1, pretty);
        }
        return outputString.str();
    }
    static std::string repeatTab(const int& count) {
        if (count <= 0) return "";
        std::string result;
        result.reserve(count);
        for (int i = 0; i < count; ++i) {
            result += "\t";
        }
        return result;
    }
    std::string getPrintableFunctions(const std::unordered_map<std::string, FunctionPointer>& functions, const int& tabs, const bool& pretty) {
        std::stringstream outputString;
        for (const auto& [name, func] : functions) {
            outputString << repeatTab(tabs) << "Function Name: " << RED_TEXT_IF(name, false) << "\n";
            outputString << repeatTab(tabs + 1) << "Function Type: " << PURPLE_TEXT_IF(format::functionTypeToString(func.function_type), pretty) << "\n";
            outputString << repeatTab(tabs + 1) << "Calling Type: " << PURPLE_TEXT_IF(format::callingTypeToString(func.calling_type), pretty) << "\n";
            outputString << repeatTab(tabs + 1) << "Return Type: " << PURPLE_TEXT_IF(format::valueTypeToString(func.return_type), pretty) << "\n";
            outputString << repeatTab(tabs + 1) << "Argument Types (" << std::to_string(func.argCount) << "):" << "\n";
            switch (func.argCount) {
            case 2:
                outputString << repeatTab(tabs + 2) << "Argument2 Type: " << LIGHT_BLUE_TEXT_IF(format::valueTypeToString(func.arg2_type), pretty) << "\n";
                [[fallthrough]];
            case 1:
                outputString << repeatTab(tabs + 2) << "Argument1 Type: " << LIGHT_BLUE_TEXT_IF(format::valueTypeToString(func.arg1_type), pretty) << "\n";
                [[fallthrough]];
            case 0:
                break;
            }
            outputString << repeatTab(tabs + 1) << "Interval: " << PURPLE_TEXT_IF((func.interval != -1 ? std::to_string(func.interval) : "Default"), pretty) << "\n";
        }
        return outputString.str();
    }
}
FindingResult findFunction(const std::unordered_map<std::string, ModuleStruct>& modules, std::string moduleName, std::string functionName) {
    FindingResult result;

    auto moduleIt = modules.find(moduleName);
    if (moduleIt != modules.end()) {
        auto& module = moduleIt->second;
        auto funcIt = module.functions.find(functionName);

        if (funcIt != module.functions.end()) {
            result.fp = funcIt->second;
            result.result_id = FindingResultID::SUCCESS;
        }
        else {
            result.result_id = FindingResultID::NO_FUNCTION;
        }
    }
    return result;
}