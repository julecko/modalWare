#pragma once

#include <string>
#include <windows.h>
#include <thread>
#include <stdexcept>
#include <iostream>
#include <any>

enum FunctionType : uint8_t {
    DEFAULT=0,
    SINGLE=1,
    THREAD=2
};
enum ValueType : int8_t {
    DEFAULT_TYPE=0,
    NONE_TYPE=1,
    INT_TYPE=2,
    CHAR_TYPE=3,
    FLOAT_TYPE=4,
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
class FunctionPointer {
private:
    bool isInitialized() const;
public:
    FunctionType function_type = FunctionType::DEFAULT;
    ValueType return_type = ValueType::DEFAULT_TYPE;
    ValueType arg1_type = ValueType::DEFAULT_TYPE;
    ValueType arg2_type = ValueType::DEFAULT_TYPE;
    int8_t argCount = -1; //Either 1 or 2
    int interval = -1;

    // Retrieves the address of the function pointer.
    void* getAddress();

    // Sets the function pointer using the function name and module handle.
    int set(const std::string& functionName, HMODULE hModule);

    // Sets the function pointer directly using a FARPROC pointer.
    int setDirect(const FARPROC& functionPointer);

    // Calls the function in specified mode
    template <typename Ret, typename... Args>
    Ret manualCall(Args... args);

    // Calls the function with specified return type and arguments.
    template <typename Ret, typename... Args>
    Ret callSingle(Args... args);

    // Calls the function in a detached thread with specified return type and arguments.
    template <typename Ret, typename... Args>
    int callInThread(Args... args);

    // Calls function with parameters automaticly
    FunctionResult autoCall(std::any arg1_val = std::any(), std::any arg2_val = std::any());
private:
    FARPROC func = nullptr;
};
