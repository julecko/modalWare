@echo off

if "%1"=="" (
    echo File for compilation missing.
    exit 1
)
set fileName=%~dp1%~n1

g++ -shared -o %fileName%.dll %fileName%.def %1 -DMYLIBRARY_EXPORTS ./ConfigManager/Config.cpp

echo "File saved in " %fileName%.dll

if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
) else (
    echo Compilation successful
)