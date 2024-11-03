#include "Config.h"

ConfigManager::ConfigManager(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        std::ofstream outFile(filename);
        outFile.close();
    }
    this->filename = filename;
}
static std::pair<std::string, std::string> split(const std::string& line, char delimiter) {
    std::istringstream stream(line);
    std::string key, value;

    std::getline(stream, key, delimiter);
    std::getline(stream, value);

    return { key, value };
}
std::unordered_map<std::string, std::string> ConfigManager::getPairs() {
    std::unordered_map<std::string, std::string> envMap;
    std::ifstream inFile(this->filename);
    std::string line;

    if (inFile.is_open()) {
        while (std::getline(inFile, line)) {
            if (!line.empty() && line.find('=') != std::string::npos) {
                auto keyValue = split(line, '=');
                envMap[keyValue.first] = keyValue.second;
            }
        }
        inFile.close();
    }
    else {
        std::cerr << "Error opening file!" << std::endl;
    }
    return envMap;
}
std::string ConfigManager::getValue(const std::string& key) {
    std::unordered_map<std::string, std::string> pairs = this->getPairs();
    auto it = pairs.find(key);
    if (it != pairs.end()) {
        return it->second;
    }
    else {
        return "";
    }
}
int ConfigManager::addValue(const std::string& key, const std::string& value) {
    std::ofstream outFile(this->filename, std::ios::app);
    if (!outFile.is_open()) {
        return 1;
    }
    outFile << key << "=" << value << std::endl;
    return 0;
}
int ConfigManager::changeValue(const std::string& key, const std::string& newValue) {
    auto values = this->getPairs();

    if (values.find(key) != values.end()) {
        values[key] = newValue;

        std::ofstream outFile(this->filename);
        if (!outFile.is_open()) {
            return 1;
        }
        for (const auto& [k, v] : values) {
            outFile << k << "=" << v << std::endl;
        }
        outFile.close();
        return 0;
    }
    return 2;
}
int ConfigManager::deleteValue(const std::string& key) {
    auto values = this->getPairs();

    values.erase(key);
    std::ofstream outFile(this->filename);
    if (!outFile.is_open()) {
        return 1;
    }
    for (const auto& [k, v] : values) {
        outFile << k << "=" << v << std::endl;
    }
    outFile.close();
    return 0;
}
int ConfigManager::renameFile(const std::string& newName) {
    try {
        std::filesystem::rename(this->filename, newName);
        this->filename = newName;
        return 0;
    }
    catch (const std::filesystem::filesystem_error& e) {
        return 1;
    }
}
int ConfigManager::deleteFile() {
    try {
        if (std::filesystem::remove(filename)) {
            return 0;
        }
        else {
            return 1;
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        return 2;
    }
}
std::filesystem::path ConfigManager::replaceFilename(const std::filesystem::path& dllPath, const std::string& newFilename) {
    std::filesystem::path parentPath = dllPath.parent_path();
    return parentPath / newFilename;
}