// main.cpp
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

class FunctionPointer {
public:
    std::string name;
    void* getAddress() {
        return (void*)this->func;
    }
    int set(const std::string& functionName, HMODULE hModule) {
        this->name = functionName;
        FARPROC addr = GetProcAddress(hModule, functionName.c_str());
        if (!addr) {
            return 1;
        }
        std::cout << (void*)addr << std::endl;
        this->func = addr;
        return 0;
    }
    int setDirect(const FARPROC& functionPointer) {
        func = functionPointer;
        if (!func) {
            return 1;
        }
        return 0;
    }
    template <typename Ret, typename... Args>
    Ret call(Args... args) {
        using FuncPtr = Ret(*)(Args...);
        FuncPtr f = reinterpret_cast<FuncPtr>(this->func);
        if (!f) {
            throw std::runtime_error("Function pointer is not set.");
        }
        return f(std::forward<Args>(args)...);
    }
private:
    FARPROC func = nullptr;
};
std::vector<FunctionPointer> listExportedFunctions(const HMODULE& hModule) {
    std::vector<FunctionPointer> functions;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;

    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "Invalid DOS signature." << std::endl;
        FreeLibrary(hModule);
        return functions;
    }

    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);

    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "Invalid NT signature." << std::endl;
        FreeLibrary(hModule);
        return functions;
    }

    DWORD exportDirRVA = pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (exportDirRVA == 0) {
        std::cerr << "No export directory found." << std::endl;
        FreeLibrary(hModule);
        return functions;
    }

    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + exportDirRVA);

    DWORD* addressesRVA = (DWORD*)((BYTE*)hModule + pExportDir->AddressOfFunctions);
    DWORD* names = (DWORD*)((BYTE*)hModule + pExportDir->AddressOfNames);
    WORD* ordinals = (WORD*)((BYTE*)hModule + pExportDir->AddressOfNameOrdinals);

    for (DWORD i = 0; i < pExportDir->NumberOfNames; ++i) {
        FunctionPointer tempFP;
        const char* currentFunctionName = (const char*)hModule + names[i];
        tempFP.name = currentFunctionName;
        WORD ordinal = ordinals[i];
        DWORD functionRVA = addressesRVA[ordinal];
        FARPROC functionAddress = (FARPROC)((BYTE*)hModule + functionRVA);

        tempFP.setDirect(functionAddress);
        functions.push_back(tempFP);
    }
    return functions;
}

int main() {
    std::string dllPath = "test.dll";
    HMODULE hModule = LoadLibraryA(dllPath.c_str());
    if (hModule == NULL) {
        std::cerr << "Failed to load the DLL: " << dllPath << std::endl;
        return 1;
    }

    std::vector<FunctionPointer> functions = listExportedFunctions(hModule);

    std::cout << "Functions exported by " << dllPath << ":" << std::endl;
    for (const auto& func : functions) {
        std::cout << func.name << std::endl;
    }
    if (functions.size() > 1) {
        functions[1].call<void>();
    }
    else {
        std::cerr << "Not enough functions found in the DLL." << std::endl;
    }
}