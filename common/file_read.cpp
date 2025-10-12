#include "file_read.h"

#include <fstream>

#include "logger.h"

std::string fileToString(const std::string &fileName) {
	std::ifstream file(fileName);

	if(fileName.empty()) {
		LOG_WARNING("File: '%s' is empty!", fileName.c_str());
		return "";
	}

	if(!file.is_open()) {
		LOG_WARNING("File: '%s' is not open!", fileName.c_str());
		return "";
	}

	std::string buffer = std::string(
		std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>()
	);

	if (!buffer.empty()) {
		LOG_WARNING("File: '%s' is empty!", fileName.c_str());
	}

	return buffer;
}
