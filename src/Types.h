#pragma once
#include <unordered_map>
#include <string>
#include <Windows.h>

// Command and Controll
enum class StatusCode : uint8_t {
	SUCCESS = 0,
	FAILURE = 1,
};
struct ReturnData {
	StatusCode result;
	std::string value;
};
// For FunctionPointer
enum class FunctionType : uint8_t {
    DEFAULT = 0,
    SINGLE = 1,
    THREAD = 2
};
enum class CallingType : uint8_t {
    DEFAULT = 0,
    MANUAL = 1,
    STARTUP = 2,
};
enum class ValueType : int8_t {
    DEFAULT_TYPE = 0,
    NONE_TYPE = 1,
    INT_TYPE = 2, 
    CHAR_TYPE = 3,
    FLOAT_TYPE = 4,
};
union Result {
    int int_result;
    char* char_result;
    float float_result;
};
struct FunctionResult {
    int8_t resultID; //Result type, -1 if error
    Result value;
};
// Module Loading
class FunctionPointer;
struct ModuleStruct {
    HMODULE hmodule;
    std::unordered_map<std::string, FunctionPointer> functions;
    std::unordered_map<std::string, std::string> metadata;
};