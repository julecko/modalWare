#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <unordered_map>
#include <cstdio>
#include <memory>
#include <array>
#include <filesystem>

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
        tempFP.setDirect(functionAddress);

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
    int initializeStatus = it->second.call<int, const char*, const char*>(VERIFICATION_CODE, location.c_str());
    std::cout << "Initialized with error code: " << initializeStatus << std::endl;
    return 0;
}
static std::vector<std::filesystem::path> getExtensionsPath() {
    std::vector<std::filesystem::path> paths;
    std::filesystem::path basePath = std::filesystem::current_path() / "modules";

    std::cout << "Searching in directory: " << basePath << std::endl;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(basePath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            paths.push_back(std::filesystem::absolute(entry.path()));
            std::cout << "Found: " << entry.path() << std::endl;
        }
    }
    return paths;
}
static std::unordered_map<std::string, ModuleStruct> loadExtensions(std::vector<std::filesystem::path> paths) {
    std::unordered_map<std::string, ModuleStruct> extensions;
    for (const std::filesystem::path path : paths) {
        ModuleStruct module;

        module.hmodule = LoadLibraryA(path.string().c_str());
        module.functions = listExportedFunctions(module.hmodule);

        std::string confFilePath = ConfigManager::replaceFilename(path, "config").string();
        initializeModul(module.functions, confFilePath);
        ConfigManager manager(confFilePath);

        std::string version = manager.getValue("version");
        if (version.empty()) {
            std::cout << "Corrupted config File -> " << path.string() << std::endl;
        }
        else {
            module.version = version;
        }

        // TODO add configuration loading

        extensions[path.filename().string()] = module;
    }
    return extensions;
}
std::unordered_map<std::string, ModuleStruct> getExtensions() {
    std::vector<std::filesystem::path> paths = getExtensionsPath();
    std::cout << "\n";
    std::unordered_map<std::string, ModuleStruct> extensions = loadExtensions(paths);
    return extensions;
}