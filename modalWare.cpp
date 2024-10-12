// main.cpp
#include <iostream>
#include <windows.h>

class FunctionPointer {
public:
    // Set the function pointer using the function name and module
    FunctionPointer(const std::string& functionName, HMODULE hModule) {
        FARPROC addr = GetProcAddress(hModule, functionName.c_str());
        if (!addr) {
            throw std::runtime_error("Could not find function: " + functionName);
        }
        func = reinterpret_cast<void*>(addr);  // Store the function pointer as a void pointer
    }

    template <typename Ret, typename... Args>
    Ret call(Args... args) {
        using FuncPtr = Ret(*)(Args...);
        FuncPtr f = reinterpret_cast<FuncPtr>(func); // Cast void* to a function pointer
        if (!f) {
            throw std::runtime_error("Function pointer is not set.");
        }
        return f(std::forward<Args>(args)...);  // Forward arguments to the function
    }

private:
    void* func = nullptr;
};

int main() {
    std::string dllName = "test.dll";
    HMODULE hModule = LoadLibraryA(dllName.c_str());

    FunctionPointer addFunction = FunctionPointer("add", hModule);
    try {
        int result = addFunction.call<int, int, int>(3, 4);
        std::cout << "The sum is: " << result << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    FunctionPointer fp;

    try {
        fp.set("hello", hModule);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    try {
        fp.call<void>();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}