#include "file_read.h"

#include <fstream>

#include "error_handle.h"

std::string fileToString(const std::string &fileName) {
	std::ifstream file(fileName);

	if(fileName.empty()) {
		printf("[VKMGL] WARNING: File: '%s' is empty!\n", fileName.c_str());
		return ""; }

	if(!file.is_open()) {
		printf("[VKMGL] ERROR: File: '%s' is not open!\n", fileName.c_str());
		return "";
	}

	std::string buffer = std::string(
		std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>()
	);

	if (!buffer.empty()) {
		printf("[VKMGL] WARNING: File: '%s' is empty!\n", fileName.c_str());
	}

	return buffer;
}
