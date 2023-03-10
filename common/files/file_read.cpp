#include "file_read.h"

#include <fstream>

#include "error/error_handle.h"

std::string fileToString(const std::string &file_name) {
	std::ifstream file(file_name);

	if(file_name.empty()) { return ""; }

	M_ASSERT(file.is_open());

	std::string buffer = std::string(
		std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>()
	);

	M_ASSERT(!buffer.empty());

	return buffer;
}
