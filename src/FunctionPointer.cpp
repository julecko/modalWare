#include "FunctionPointer.h"

void* FunctionPointer::getAddress() {
    return (void*)this->func;
}
int FunctionPointer::set(const std::string& functionName, HMODULE hModule) {
    this->name = functionName;
    FARPROC addr = GetProcAddress(hModule, functionName.c_str());
    if (!addr) {
        return 1;
    }
    std::cout << (void*)addr << std::endl;
    this->func = addr;
    return 0;
}
int FunctionPointer::setDirect(const FARPROC& functionPointer) {
    func = functionPointer;
    if (!func) {
        return 1;
    }
    return 0;
}
template <typename Ret, typename... Args>
Ret FunctionPointer::call(Args... args) const {
    using FuncPtr = Ret(*)(Args...);
    FuncPtr f = reinterpret_cast<FuncPtr>(this->func);
    if (!f) {
        throw std::runtime_error("Function pointer is not set.");
    }
    return f(std::forward<Args>(args)...);
}
template <typename Ret, typename... Args>
void FunctionPointer::callInThread(Args... args) const {
    std::thread([this, args...]() {
        try {
            this->call<Ret>(args...);
        }
        catch (const std::exception& ex) {
            std::cerr << "Error calling function in thread: " << ex.what() << std::endl;
        }
        }).detach();
}
template int FunctionPointer::call<int, const char*, const char*>(const char*, const char*) const;
