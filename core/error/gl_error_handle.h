#pragma once

#ifdef _WIN32
	#include <windows.h>
#endif

#include <glad/gl.h>

void printGLErrors(const char* file, int line, const char* function);

#define MY_GL_CHECK(MyFunction)                   \
	MyFunction;                                   \
	printGLErrors(__FILE__, __LINE__, __FUNCTION__)
