// main.cpp
#include <windows.h>
#include <iostream>
#include <unordered_map>

#include "./ConfigManager/Config.h"
#include "./src/CommandsAndControll.h"
#include "./src/ModuleLoading.h"
#include "./src/FunctionPointer.h"
#include "./src/Types.h"

//Global variables
FunctionPointer get_message;
std::unordered_map<std::string, ModuleStruct> extensions;

static int startup(const std::unordered_map<std::string, FunctionPointer>& functions) {
    for (const auto& funcEntry : functions) {
        const FunctionPointer& functionPtr = funcEntry.second;

        if (functionPtr.calling_type == CallingType::STARTUP) {
            FunctionResult result = functionPtr.autoCall();
            std::cout << "Startup application -> " << funcEntry.first << " and the result is: " << static_cast<int>(result.resultID) << std::endl; //Debug
        }
    }
    return 1;
}
static void loop() {
    while (true) {
        FunctionResult result = get_message.autoCall();
        std::cout << "Result is -> " << static_cast<int>(result.resultID) << "~";

        if (result.value.char_result != nullptr) {
            std::cout << result.value.char_result << std::endl;
        }
        else {
            std::cout << "char_result is null!" << std::endl;
        }
        Sleep(2000);
    }
}
int main() {
    extensions = getExtensions();

    for (const auto& [moduleName, moduleStruct] : extensions) {
        std::cout << "Loaded module: " << moduleName << std::endl;
        startup(moduleStruct.functions);
        for (const auto& [functionName, functionPointer] : moduleStruct.functions) {
            if (functionName == "get_message") {
                get_message = functionPointer;
                std::cout << "Get message found" << std::endl;
            }
            std::cout << "\tFunction: " << functionName << std::endl;
        }
    }
    
    std::cout << "END" << std::endl;
    return 0;
}