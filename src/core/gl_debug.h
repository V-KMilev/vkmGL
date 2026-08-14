#pragma once

#include <GL/glew.h>

namespace Core {
/**
 * @brief Enable OpenGL debug output with a default callback that logs messages.
 *
 * Requires GL_KHR_debug support (GL 4.3+ or extension). Safe to call; no-ops if
 * glDebugMessageCallback is not available.
 *
 * @param synchronous If true, enables GL_DEBUG_OUTPUT_SYNCHRONOUS for immediate callbacks.
 */
void enableGLDebugLogging(bool synchronous = true);

} // namespace Core

