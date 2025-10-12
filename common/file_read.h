#pragma once

#include <string>

/**
 * @brief Reads the contents of a file into a string.
 * 
 * Opens the specified file and returns its contents as a std::string. If the
 * file does not exist, cannot be opened, or is empty, an empty string is returned.
 * 
 * @param fileName The path to the file to read.
 * @return The file contents as a std::string, or an empty string on failure.
 */
std::string fileToString(const std::string &fileName);
