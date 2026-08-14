#pragma once

#include <GL/glew.h>

/**
 * @brief Drain the OpenGL error queue, logging and aborting if it is not empty.
 *
 * GL queues errors rather than holding one, so this loops until the queue is
 * empty and reports every entry - stopping at the first would leave the rest to
 * surface at some unrelated later check.
 *
 * @param file     Source file the check is performed from.
 * @param line     Line number in that file.
 * @param function Name of the enclosing function.
 */
void checkGLError(
    const char* file,
    int line,
    const char* function
);

/**
 * @brief Run an OpenGL call and check the error queue immediately after it.
 *
 * A statement, not an expression: the body is wrapped so that
 * `if (cond) VKM_GL_CHECK(glFoo());` binds the check to the branch instead of
 * running it unconditionally. A call whose result is needed assigns inside the
 * macro rather than around it:
 *
 * @code
 * uint32_t id = 0;
 * VKM_GL_CHECK(id = glCreateShader(type));
 * @endcode
 *
 * Release builds (NDEBUG) drop the check but keep the same statement shape, so
 * control flow cannot differ between configurations.
 *
 * @param GL_CALL The OpenGL call to execute and check.
 */
#ifdef NDEBUG
    #define VKM_GL_CHECK(GL_CALL) \
        do {                      \
            GL_CALL;              \
        } while (0)
#else
    #define VKM_GL_CHECK(GL_CALL)                              \
        do {                                                   \
            GL_CALL;                                           \
            checkGLError(__FILE__, __LINE__, __FUNCTION__);    \
        } while (0)
#endif
