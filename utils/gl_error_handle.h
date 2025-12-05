#pragma once

#include <glad/gl.h>

/**
 * @brief Checks for any OpenGL errors and logs detailed information.
 * 
 * This function checks the OpenGL error state and, if an error is present,
 * logs a detailed message including the file name, line number, and function name.
 *
 * @param file The source file where the check is being performed.
 * @param line The line number in the source file.
 * @param function The name of the function from which the check is called.
 */
void checkGLError(
	const char* file,
	int line,
	const char* function
);

/**
 * @brief Executes an OpenGL function and checks for any GL errors.
 *
 * This macro calls the specified OpenGL function and immediately checks for
 * any errors that may have occurred during its execution. If an error is found,
 * a detailed log message is produced including the file name, line number, and function name.
 *
 * @param MyFunction The OpenGL function to be executed and checked.
 */
#define VKM_GL_CHECK(MyFunction)                  \
	MyFunction;                                   \
	checkGLError(__FILE__, __LINE__, __FUNCTION__)
