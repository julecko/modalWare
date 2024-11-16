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
    catch (const std::filesystem::filesystem_error&) {
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
    catch (const std::filesystem::filesystem_error&) {
        return 2;
    }
}
int ConfigManager::writeFunction(const FunctionData& data) {
    if (!this->allowFunc) {
        return 1;
    }
    std::ofstream outFile(this->filename, std::ios::app);

    outFile << data.name << "<" << data.returnType;

    if (!data.argTypes.empty()) {
        outFile << ", ";
        for (size_t i = 0; i < data.argTypes.size(); ++i) {
            outFile << data.argTypes[i];
            if (i < data.argTypes.size() - 1) {
                outFile << ", ";
            }
        }
    }
    outFile << "> ";

    outFile << data.funcType;

    if (data.interval != -1) {
        outFile << "~" << data.interval;
    }

    outFile << std::endl;
    return 0;
}
void ConfigManager::initializeFunctionWriting() {
    std::ofstream outFile(this->filename, std::ios::app);
    outFile << "Functions" << std::endl;
    this->allowFunc = true;
}
std::vector<std::string> splitTypes(const std::string& str) {
    std::vector<std::string> types;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ',')) {
        token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
        token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), token.end());
        types.push_back(token);
    }
    return types;
}
FunctionData ConfigManager::processFunctionLine(const std::string& line) {
    const std::regex functionRegex(R"((\w+)<([^>]+)> (\w+)(?:~(\d+))?)");
    std::smatch match;
    FunctionData info;

    if (std::regex_match(line, match, functionRegex)) {
        info.name = match[1];
        std::vector<std::string> types = splitTypes(match[2]);
        info.returnType = types.front();
        types.erase(types.begin());
        info.argTypes = types;
        info.funcType = match[3];

        if (match[4].matched) {
            info.interval = std::stoi(match[4]);
        }
    }
    return info;
}
std::filesystem::path ConfigManager::replaceFilename(const std::filesystem::path& dllPath, const std::string& newFilename) {
    std::filesystem::path parentPath = dllPath.parent_path();
    return parentPath / newFilename;
}