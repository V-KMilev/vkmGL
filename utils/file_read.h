#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "logger.h"

/**
 * @brief Read entire file contents into a string
 * @param filepath Path to the file
 * @return File contents as string, empty string if failed
 */
inline std::string fileToString(const std::string& filepath) {
	if (filepath.empty()) {
		LOG_WARNING("File path is empty");
		return "";
	}

	std::ifstream file(filepath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		LOG_WARNING("Failed to open file: %s", filepath.c_str());
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

