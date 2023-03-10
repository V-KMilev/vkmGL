#pragma once

#include <string>

/**
 * @brief Reads the given file by seeking through it.
 * 
 * @note There is a check if the file_name is valid or not. 
 * We get the  size of the file by seeing to the end of it 
 * and ftell(). The buffer we return is been filled by fread() 
 * 
 * @param[in] file_name 
 * @return std::string 
 */
std::string fileToString(const std::string &file_name);
