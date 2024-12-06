#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include <vector>
#include <regex>

struct FunctionData {
	std::string name;
	std::string returnType;
	std::vector<std::string> argTypes; // Max 2, min 0
	std::string callType;
	std::string funcType;
	int interval = -1;
};

class ConfigManager {
public:
	ConfigManager(const std::string& filename);

	/**
	 * @brief Get config data.
	 *
	 * Fetches all data from config file in format key=value
	 *
	 * @return Returns hashmap of config file
	 */
	std::unordered_map<std::string, std::string> getPairs();

	/**
	* @brief Get single value
	* 
	* Gets all values using getValues and chooses only one
	* 
	* @return Returns single value
	*/
	std::string getValue(const std::string& key);

	/**
	 * @brief Writes new pair of key, value into config file
	 *
	 * @return Returns 0 when the value was written successfully, 1 on failure.
	 */
	int addValue(const std::string& key, const std::string& value);

	/**
	* @brief Change value in config file
	* 
	* @return Returns 0 on success, 1 if file cannot be open and 2 if key doesnt exists
	*/
	int changeValue(const std::string& key, const std::string& newValue);

	/**
	* @brief Delete file from config file
	*
	* @return Returns 0 on success, 1 if file cannot be open
	*/
	int deleteValue(const std::string& key);

	/**
	* @brief Renames config file
	*
	* @return Returns 0 on success, 1 on failure
	*/
	int renameFile(const std::string& newName);

	/**
	* @brief Deletes config file
	* 
	* @return Returns 0 if successful, 1 on failure
	*/
	int deleteFile();

	/**
	* @brief Allow functions to be written in file
	*
	* @return Returns 0 if successful, 1 on failure
	*/
	void initializeFunctionWriting();

	/**
	* @brief Writes configuration for function into config file
	*
	* @return Returns 0 if successful, 1 on failure
	*/
	int writeFunction(const FunctionData& data);

	/**
	* @brief Extracts data from configuration files line
	*
	* @return Returns data about function
	*/
	static FunctionData processFunctionLine(const std::string& line);

	/**
	* @brief Replaces filename in path
	*
	* @return Returns new filename
	*/
	static std::filesystem::path replaceFilename(const std::filesystem::path& dllPath, const std::string& newFilename);
private:
	std::string filename;
	bool allowFunc;
};