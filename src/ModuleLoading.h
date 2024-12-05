#pragma once
#include <string>
#include <windows.h>
#include <filesystem>
#include <unordered_map>
#include "Types.h"

std::unordered_map<std::string, ModuleStruct> getExtensions();