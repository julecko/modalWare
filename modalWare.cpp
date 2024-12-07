// main.cpp
#include <windows.h>
#include <iostream>
#include <unordered_map>

#include "./ConfigManager/Config.h"
#include "./src/CommandsAndControll.h"
#include "./src/ModuleLoading.h"
#include "./src/FunctionPointer.h"
#include "./src/Util.h"
#include "./src/Types.h"

//Global variables
FunctionPointer get_message;
std::unordered_map<std::string, ModuleStruct> modules;

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
            std::cout << "Message is empty" << std::endl;
        }
        Sleep(2000);
    }
}
int main() {
    modules = getExtensions();
    get_message = findFunction(modules, "discord_bot.dll", "get_message").fp;
    for (const auto& [moduleName, moduleStruct] : modules) {
        startup(moduleStruct.functions);
    }
    std::cout << format::getPrintableModules(modules, true);
    std::cout << static_cast<int>(findFunction(modules, "discord_bot.dll", "send_message").fp.autoCall(format::getPrintableModules(modules).c_str()).value.int_result) << std::endl;
    loop();
    return 0;
}