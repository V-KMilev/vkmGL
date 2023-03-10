#pragma once

#include <iostream>

#ifdef _WIN32
	#include <windows.h>
#endif

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/**
 * @brief Private function called only by the macro 
 * MY_GL_CHECK used for printing the GL error
 * 
 * @note printGLErrors print is been called only if 
 * the given glGetError() is a valid error.__FILE__, 
 * __LINE__ and __FUNCTION__ are equal of the file, 
 * line and funtion the MY_GL_CHECK is being executed.
 * 
 * @param[in] file 
 * @param[in] line 
 * @param[in] function 
 */
void printGLErrors(const char* file, int line, const char* function);

#define MY_GL_CHECK(MyFunction)                     \
	MyFunction;                                     \
	printGLErrors(__FILE__, __LINE__, __FUNCTION__)
