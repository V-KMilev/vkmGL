#include "gl_debug.h"

#include <string>

#include "gl_error_handle.h"
#include "logger.h"

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef GLAPIENTRY
#define GLAPIENTRY APIENTRY
#endif

namespace Core {
namespace {
    void GLAPIENTRY glDebugCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
    ) {
        (void)length;
        (void)userParam;

        // Filter out noisy non-significant messages if desired
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
            return;
        }

        const char* srcStr = "OTHER";
        switch (source) {
            case GL_DEBUG_SOURCE_API:             srcStr = "API"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   srcStr = "WINDOW_SYSTEM"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: srcStr = "SHADER_COMPILER"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     srcStr = "THIRD_PARTY"; break;
            case GL_DEBUG_SOURCE_APPLICATION:     srcStr = "APPLICATION"; break;
            case GL_DEBUG_SOURCE_OTHER:           srcStr = "OTHER"; break;
        }

        const char* typeStr = "OTHER";
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               typeStr = "ERROR"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "DEPRECATED_BEHAVIOR"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "UNDEFINED_BEHAVIOR"; break;
            case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "PORTABILITY"; break;
            case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "PERFORMANCE"; break;
            case GL_DEBUG_TYPE_MARKER:              typeStr = "MARKER"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "PUSH_GROUP"; break;
            case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "POP_GROUP"; break;
            case GL_DEBUG_TYPE_OTHER:               typeStr = "OTHER"; break;
        }

        const char* sevStr = "NOTIFICATION";
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:         sevStr = "HIGH"; break;
            case GL_DEBUG_SEVERITY_MEDIUM:       sevStr = "MEDIUM"; break;
            case GL_DEBUG_SEVERITY_LOW:          sevStr = "LOW"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: sevStr = "NOTIFICATION"; break;
        }

        LOG_ERROR("[GL DEBUG][%s][%s][%s][ID:%u] %s", srcStr, typeStr, sevStr, id, message);
    }
} // namespace

void enableGLDebugLogging(bool synchronous) {
#ifdef GL_DEBUG_OUTPUT
    if (!glDebugMessageCallback) {
        LOG_WARNING("GL debug output not available (glDebugMessageCallback missing)");
        return;
    }

    VKM_GL_CHECK(glEnable(GL_DEBUG_OUTPUT));
    if (synchronous) {
        VKM_GL_CHECK(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    }

    VKM_GL_CHECK(glDebugMessageCallback(glDebugCallback, nullptr));
    // Optionally filter messages: here we keep all severities except we early-return notifications above.
    LOG_INFO("GL debug output enabled%s", synchronous ? " (synchronous)" : "");
#else
    LOG_WARN("GL_DEBUG_OUTPUT not supported at compile time");
    (void)synchronous;
#endif
}
} // namespace Core

