// main.cpp
#include <windows.h>
#include <iostream>
#include <unordered_map>

#include "./ConfigManager/Config.h"
#include "./src/CommandsAndControll.h"
#include "./src/ModuleLoading.h"


int main() {
    std::unordered_map<std::string, ModuleStruct> extensions = getExtensions();

    for (const auto& [moduleName, moduleStruct] : extensions) {
        std::cout << "Loaded module: " << moduleName << std::endl;

        for (const auto& [functionName, functionPointer] : moduleStruct.functions) {
            std::cout << "\tFunction: " << functionName << std::endl;
        }
    }
    return 0;
}