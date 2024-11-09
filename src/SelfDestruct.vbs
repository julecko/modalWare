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