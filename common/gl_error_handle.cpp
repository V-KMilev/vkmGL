#include "gl_error_handle.h"

#include <stdio.h>
// Linux g++ specific for abort()
#include <stdlib.h>

#include "logger.h"

void checkGLError(
    const char* file,
    int line,
    const char* function
) {
    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        LOG_ERROR("VKM GL CHECK Failed!\t[%s:%d][F:%s] Error: %ud", file, line, function, error);

#ifdef _WIN32
        __debugbreak();
#else
        abort();
#endif
    }
}
