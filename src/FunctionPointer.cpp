#include "FunctionPointer.h"

//Here will be some function templates used for standard functions
template int FunctionPointer::manualCall<int, const char*, const char*>(const char*, const char*) const;
template int FunctionPointer::manualCall<int, int, int>(int, int) const;

void* FunctionPointer::getAddress() {
    return (void*)this->func;
}
int FunctionPointer::set(const std::string& functionName, HMODULE hModule) {
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
Ret FunctionPointer::manualCall(Args... args) const {
    if (this->function_type == FunctionType::THREAD) {
        callInThread<Ret>(std::forward<Args>(args)...);
        return Ret();
    }
    return callSingle<Ret>(std::forward<Args>(args)...);
}
template <typename Ret, typename... Args>
Ret FunctionPointer::callSingle(Args... args) const {
    using FuncPtr = Ret(*)(Args...);
    FuncPtr f = reinterpret_cast<FuncPtr>(this->func);
    if (!f) {
        return Ret();
    }
    return f(std::forward<Args>(args)...);
}
template <typename Ret, typename... Args>
int FunctionPointer::callInThread(Args... args) const {
    try {
        std::thread([this, args...]() {
            try {
                this->callSingle<Ret>(args...);
            }
            catch (const std::exception& ex) {
                std::cerr << "Thread exception: " << ex.what() << std::endl;
            }
            }).detach();
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to launch thread: " << ex.what() << std::endl;
        return 1;
    }
    std::cout << "Thread exited" << std::endl;
    return 0;
}
FunctionResult FunctionPointer::autoCall(std::any arg1_val, std::any arg2_val) const {
    FunctionResult result{};
    if (!isInitialized()) {
        result.resultID = -1;
        return result;
    }
    result.resultID = static_cast<int8_t>(this->return_type);
    if (function_type == FunctionType::THREAD) {
        if (argCount == 0) {
            this->callInThread<void>();
        }
        else if (argCount == 1) {
            if (arg1_type == ValueType::INT_TYPE) {
                int arg1 = std::any_cast<int>(arg1_val);
                result.value.int_result = this->callInThread<void, int>(arg1);
            }
            else if (arg1_type == ValueType::CHAR_TYPE) {
                const char* arg1 = std::any_cast<const char*>(arg1_val);
                result.value.int_result = this->callInThread<void, const char*>(arg1);
            }
            else if (arg1_type == ValueType::FLOAT_TYPE) {
                float arg1 = std::any_cast<float>(arg1_val);
                result.value.int_result = this->callInThread<void, float>(arg1);
            }
            else {
                result.resultID = -1;
            }
        }
        else if (argCount == 2) {
            if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::INT_TYPE) {
                int arg1 = std::any_cast<int>(arg1_val);
                int arg2 = std::any_cast<int>(arg2_val);
                result.value.int_result = this->callInThread<void, int, int>(arg1, arg2);
            }
            else if (arg1_type == ValueType::CHAR_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                const char* arg1 = std::any_cast<const char*>(arg1_val);
                const char* arg2 = std::any_cast<const char*>(arg2_val);
                result.value.int_result = this->callInThread<void, const char*, const char*>(arg1, arg2);
            }
            else if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                int arg1 = std::any_cast<int>(arg1_val);
                const char* arg2 = std::any_cast<const char*>(arg2_val);
                result.value.int_result = this->callInThread<void, int, const char*>(arg1, arg2);
            }
            else {
                result.resultID = -1;
            }
        }
    }
    else if (function_type == FunctionType::SINGLE) {
        if (return_type == ValueType::NONE_TYPE) {
            if (argCount == 0) {
                this->callSingle<void>();
            }
            else if (argCount == 1) {
                if (arg1_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    this->callSingle<void, int>(arg1);
                }
                else if (arg1_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    this->callSingle<void, const char*>(arg1);
                }
                else if (arg1_type == ValueType::FLOAT_TYPE) {
                    float arg1 = std::any_cast<float>(arg1_val);
                    this->callSingle<void, float>(arg1);
                }
                else {
                    result.resultID = -1;
                }
            }
            else if (argCount == 2) {
                if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    int arg2 = std::any_cast<int>(arg2_val);
                    this->callSingle<void, int, int>(arg1, arg2);
                }
                else if (arg1_type == ValueType::CHAR_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    this->callSingle<void, const char*, const char*>(arg1, arg2);
                }
                else if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    this->callSingle<void, int, const char*>(arg1, arg2);
                }
                else {
                    result.resultID = -1;
                }
            }
        }
        else if (return_type == ValueType::INT_TYPE) {
            if (argCount == 0) {
                result.value.int_result = this->callSingle<int>();
            }
            else if (argCount == 1) {
                if (arg1_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    result.value.int_result = this->callSingle<int, int>(arg1);
                }
                else if (arg1_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    result.value.int_result = this->callSingle<int, const char*>(arg1);
                }
                else if (arg1_type == ValueType::FLOAT_TYPE) {
                    float arg1 = std::any_cast<float>(arg1_val);
                    result.value.int_result = this->callSingle<int, float>(arg1);
                }
                else {
                    result.resultID = -1;
                }
            }
            else if (argCount == 2) {
                if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    int arg2 = std::any_cast<int>(arg2_val);
                    result.value.int_result = this->callSingle<int, int, int>(arg1, arg2);
                }
                else if (arg1_type == ValueType::CHAR_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    result.value.int_result = this->callSingle<int, const char*, const char*>(arg1, arg2);
                }
                else if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    result.value.int_result = this->callSingle<int, int, const char*>(arg1, arg2);
                }
                else {
                    result.resultID = -1;
                }
            }
        }
        else if (return_type == ValueType::CHAR_TYPE) {
            if (argCount == 0) {
                result.value.char_result = this->callSingle<char*>();
            }
            else if (argCount == 1) {
                if (arg1_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    result.value.char_result = this->callSingle<char*, int>(arg1);
                }
                else if (arg1_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    result.value.char_result = this->callSingle<char*, const char*>(arg1);
                }
                else if (arg1_type == ValueType::FLOAT_TYPE) {
                    float arg1 = std::any_cast<float>(arg1_val);
                    result.value.char_result = this->callSingle<char*, float>(arg1);
                }
                else {
                    result.resultID = -1;
                }
            }
            else if (argCount == 2) {
                if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    int arg2 = std::any_cast<int>(arg2_val);
                    result.value.char_result = this->callSingle<char*, int, int>(arg1, arg2);
                }
                else if (arg1_type == ValueType::CHAR_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    result.value.char_result = this->callSingle<char*, const char*, const char*>(arg1, arg2);
                }
                else if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    result.value.char_result = this->callSingle<char*, int, const char*>(arg1, arg2);
                }
                else {
                    result.resultID = -1;
                }
            }
        }
        else if (return_type == ValueType::FLOAT_TYPE) {
            if (argCount == 0) {
                result.value.float_result = this->callSingle<float>();
            }
            else if (argCount == 1) {
                if (arg1_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    result.value.float_result = this->callSingle<float, int>(arg1);
                }
                else if (arg1_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    result.value.float_result = this->callSingle<float, const char*>(arg1);
                }
                else if (arg1_type == ValueType::FLOAT_TYPE) {
                    float arg1 = std::any_cast<float>(arg1_val);
                    result.value.float_result = this->callSingle<float, float>(arg1);
                }
                else {
                    result.resultID = -1;
                }
            }
            else if (argCount == 2) {
                if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::INT_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    int arg2 = std::any_cast<int>(arg2_val);
                    result.value.float_result = this->callSingle<float, int, int>(arg1, arg2);
                }
                else if (arg1_type == ValueType::CHAR_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    const char* arg1 = std::any_cast<const char*>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    result.value.float_result = this->callSingle<float, const char*, const char*>(arg1, arg2);
                }
                else if (arg1_type == ValueType::INT_TYPE && arg2_type == ValueType::CHAR_TYPE) {
                    int arg1 = std::any_cast<int>(arg1_val);
                    const char* arg2 = std::any_cast<const char*>(arg2_val);
                    result.value.float_result = this->callSingle<float, int, const char*>(arg1, arg2);
                }
                else {
                    result.resultID = -1;
                }
            }
        }
    }
    return result;
}
bool FunctionPointer::isInitialized() const {
    return !(
        this->function_type == FunctionType::DEFAULT ||
        this->calling_type == CallingType::DEFAULT ||
        this->return_type == ValueType::DEFAULT_TYPE ||
        this->argCount == -1 ||
        this->func == nullptr);
}