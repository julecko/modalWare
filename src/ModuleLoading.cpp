#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <cstdio>
#include <memory>
#include <array>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

#include "Types.h"
#include "Util.h"
#include "ModuleLoading.h"
#include "FunctionPointer.h"
#include "../ConfigManager/Config.h"

#define VERIFICATION_CODE "12234"

static std::unordered_map<std::string, FunctionPointer> listExportedFunctions(const HMODULE& hModule) {
    std::unordered_map<std::string, FunctionPointer> functionMap;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;

    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "Invalid DOS signature." << std::endl;
        FreeLibrary(hModule);
        return functionMap;
    }

    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);

    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "Invalid NT signature." << std::endl;
        FreeLibrary(hModule);
        return functionMap;
    }

    DWORD exportDirRVA = pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (exportDirRVA == 0) {
        std::cerr << "No export directory found." << std::endl;
        FreeLibrary(hModule);
        return functionMap;
    }

    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + exportDirRVA);

    DWORD* addressesRVA = (DWORD*)((BYTE*)hModule + pExportDir->AddressOfFunctions);
    DWORD* names = (DWORD*)((BYTE*)hModule + pExportDir->AddressOfNames);
    WORD* ordinals = (WORD*)((BYTE*)hModule + pExportDir->AddressOfNameOrdinals);

    for (DWORD i = 0; i < pExportDir->NumberOfNames; ++i) {
        const char* currentFunctionName = (const char*)hModule + names[i];
        WORD ordinal = ordinals[i];
        DWORD functionRVA = addressesRVA[ordinal];
        FARPROC functionAddress = (FARPROC)((BYTE*)hModule + functionRVA);

        FunctionPointer tempFP;
        if (tempFP.setDirect(functionAddress) != 0) {
            std::cerr << "Failed to set function pointer for: " << currentFunctionName << std::endl;
            continue;
        }

        functionMap[currentFunctionName] = tempFP;
    }
    return functionMap;
}
static int initializeModul(std::unordered_map<std::string, FunctionPointer>& functions, std::string location) {
    auto it = functions.find("initialize");
    if (it == functions.end()) {
        std::cout << "Error initializing" << std::endl;
        return 1;
    }
    int initializeStatus = it->second.manualCall<int, const char*, const char*>(VERIFICATION_CODE, location.c_str());
    std::cout << "Initialized with error code: " << initializeStatus << std::endl;
    return 0;
}
static std::vector<std::filesystem::path> getExtensionsPath() {
    std::vector<std::filesystem::path> paths;
    std::filesystem::path basePath = std::filesystem::current_path() / "modules";
    if (!std::filesystem::exists(basePath)) {
        try {
            if (std::filesystem::create_directories(basePath)) {
                std::cout << "Directory created: " << basePath << std::endl;
            }
            else {
                std::cerr << "Failed to create directory: " << basePath << std::endl;
                return paths;
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            return paths;
        }
    }
    else {
        std::cout << "Directory already exists: " << basePath << std::endl;
    }

    std::cout << "Searching in directory: " << basePath << std::endl;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(basePath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            paths.push_back(std::filesystem::absolute(entry.path()));
            std::cout << "Found: " << entry.path() << std::endl;
        }
    }
    return paths;
}
static std::unordered_map<std::string, ModuleStruct> loadExtensions(const std::vector<std::filesystem::path>& paths) {
    std::unordered_map<std::string, ModuleStruct> extensions;
    for (const std::filesystem::path path : paths) {
        ModuleStruct module;

        HMODULE hModule = LoadLibraryA(path.string().c_str());
        if (!hModule) {
            DWORD error = GetLastError();
            std::cerr << "Failed to load module: " << path.string() << " (Error Code: " << error << ")" << std::endl;
            continue;
        }
        module.hmodule = hModule;
        module.functions = listExportedFunctions(module.hmodule);

        std::string confFilePath = ConfigManager::replaceFilename(path, "config").string();
        initializeModul(module.functions, confFilePath);

        ConfigManager manager(confFilePath);
        std::string version = manager.getValue("version");
        if (version.empty()) {
            std::cout << "Corrupted config File -> " << path.string() << std::endl;
        }
        module.metadata = manager.getPairs();

        // TODO add configuration loading

        extensions[path.filename().string()] = module;
    }
    return extensions;
}
template <typename T>
static std::unordered_set<T> symetric_difference(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
    std::unordered_set<T> result;

    for (const auto& elem : set1) {
        if (set2.find(elem) == set2.end()) {
            result.insert(elem);
        }
    }
    for (const auto& elem : set2) {
        if (set1.find(elem) == set1.end()) {
            result.insert(elem);
        }
    }
    result.erase("initialize");
    return result;
}
static ValueType convertValueType(const std::string& value) {
    if (value == "int") {
        return ValueType::INT_TYPE;
    }
    else if (value == "void") {
        return ValueType::NONE_TYPE;
    }
    else if (value == "char") {
        return ValueType::CHAR_TYPE;
    }
    else if (value == "float") {
        return ValueType::FLOAT_TYPE;
    }
    else {
        return ValueType::DEFAULT_TYPE;
    }
}
static CallingType convertCallingType(const std::string& value) {
    if (value == "manual") {
        return CallingType::MANUAL;
    }
    else if (value == "startup") {
        return CallingType::STARTUP;
    }
    else {
        return CallingType::DEFAULT;
    }
}
static int loadFunctionsConfig(const std::filesystem::path& dllPath, std::unordered_map<std::string, FunctionPointer>& functions) {
    std::filesystem::path confPath = ConfigManager::replaceFilename(dllPath, "config");
    // First do checks
    if (!std::filesystem::exists(confPath)) {
        std::cout << "Path doesn't exist" << std::endl;
        return 1;
    }

    std::ifstream file(confPath);
    std::vector<FunctionData> functionsInfo;
    std::string line;
    bool parsingFunctions = false;

    std::unordered_set<std::string> originalNames;
    std::unordered_set<std::string> newNames;

    for (const auto& pair : functions) {
        originalNames.insert(pair.first);
    }

    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (!parsingFunctions && line == "Functions") {
            parsingFunctions = true;
            continue;
        }

        if (!parsingFunctions) {
            continue;
        }
        FunctionData data;
        try {
            data = ConfigManager::processFunctionLine(line);
        }
        catch (const std::exception& e) {
            continue;
        }
        if (data.name.empty()) {
            continue;
        }

        newNames.insert(data.name);
        auto it = functions.find(data.name);
        if (it == functions.end()) {
            std::cout << "Function " << data.name << "not found" << std::endl;
            continue;
        }
        FunctionPointer& tempFunc = it->second;

        tempFunc.function_type = (data.funcType == "single") ? FunctionType::SINGLE : FunctionType::THREAD;
        tempFunc.argCount = static_cast<int8_t>(data.argTypes.size());

        tempFunc.interval = data.interval;
        tempFunc.return_type = convertValueType(data.returnType);
        tempFunc.calling_type = convertCallingType(data.callType);

        switch (tempFunc.argCount) {
        case 2:
            tempFunc.arg2_type = convertValueType(data.argTypes.at(1));
        case 1:
            tempFunc.arg1_type = convertValueType(data.argTypes.at(0));
        case 0:
            break;
        default:
            std::cout << "Too many arguments, aborting" << std::endl;
        }

        functionsInfo.push_back(data);
    }
    file.close();
    std::unordered_set unusedFunctions = symetric_difference<std::string>(originalNames, newNames);
    for (const auto& funcName : unusedFunctions) {
        std::cout << "Unused function: " << funcName << std::endl;
    }
    return 0;
}
std::unordered_map<std::string, ModuleStruct> getExtensions() {
    std::vector<std::filesystem::path> paths = getExtensionsPath();
    std::unordered_map<std::string, ModuleStruct> extensions = loadExtensions(paths);
    for (const auto& path : paths) {
        auto it = extensions.find(path.filename().string());
        if (it != extensions.end()) {
            auto& functions = it->second.functions;
        }
        else {
            continue; //TODO add later error handling
        }
        loadFunctionsConfig(path, it->second.functions);
    }
    return extensions;
}