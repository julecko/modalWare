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
std::unordered_map<std::string, std::string> ConfigManager::getValues() {
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
int ConfigManager::writeValue(const std::string& key, const std::string& value) {
    std::ofstream outFile(this->filename, std::ios::app);
    if (!outFile.is_open()) {
        return 1;
    }
    outFile << key << "=" << value << std::endl;
    return 0;
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