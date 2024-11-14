#pragma once

#include <string>
#include <windows.h>
#include <thread>
#include <stdexcept>
#include <iostream>

class FunctionPointer {
public:
    std::string name;

    // Retrieves the address of the function pointer.
    void* getAddress();

    // Sets the function pointer using the function name and module handle.
    int set(const std::string& functionName, HMODULE hModule);

    // Sets the function pointer directly using a FARPROC pointer.
    int setDirect(const FARPROC& functionPointer);

    // Calls the function with specified return type and arguments.
    template <typename Ret, typename... Args>
    Ret call(Args... args) const;

    // Calls the function in a detached thread with specified return type and arguments.
    template <typename Ret, typename... Args>
    void callInThread(Args... args) const;

private:
    FARPROC func = nullptr;
};
