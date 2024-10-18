#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <filesystem>


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
	std::unordered_map<std::string, std::string> getValues();

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
private:
	std::string filename;
};