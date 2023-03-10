#include "gl_error_handle.h"

void printGLErrors(
	const char* file,
	int line,
	const char* function
) {
	GLenum error = glGetError();

	if (error != GL_NO_ERROR) {
		std::cerr << "MY_GL_CHECK FAILED: "
			<< function << " [ERROR:" << error << "] at [" << file << ":" << line << "]\n";
#ifdef _WIN32
		__debugbreak();
#endif
	}
}
