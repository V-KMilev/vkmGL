#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "DEBUG"

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
        LOG_ERROR("VKM_GL_CHECK failed at %s:%d in %s: GL error 0x%X", file, line, function, error);

#ifdef _WIN32
        __debugbreak();
#else
        abort();
#endif
    }
}
