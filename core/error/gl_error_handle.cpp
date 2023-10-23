#include "gl_error_handle.h"

#include <stdio.h>

void printGLErrors(
	const char* file,
	int line,
	const char* function
) {
	GLenum error = glGetError();

	if (error != GL_NO_ERROR) {
		printf("[FAILED] MY_GL_CHECK >>\t[%s:%d][F:%s] Error: %s\n", file, line, function, error);

#ifdef _WIN32
		__debugbreak();
#else
		abort();
#endif
	}
}
