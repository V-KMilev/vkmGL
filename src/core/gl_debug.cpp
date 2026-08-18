#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "DEBUG"

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

namespace Vkm::GL {
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

        // Severity maps to log level so HIGH GL errors log as ERROR and
        // MEDIUM as WARNING. Previously everything came out as ERROR, which
        // turned routine driver hints into noise.
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
                LOG_ERROR("[%s/%s/HIGH][ID:%u] %s", srcStr, typeStr, id, message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                LOG_WARNING("[%s/%s/MED][ID:%u] %s", srcStr, typeStr, id, message);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                LOG_INFO("[%s/%s/LOW][ID:%u] %s", srcStr, typeStr, id, message);
                break;
            default:
                LOG_VERBOSE("[%s/%s][ID:%u] %s", srcStr, typeStr, id, message);
                break;
        }
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
    LOG_WARNING("GL_DEBUG_OUTPUT not supported at compile time");
    (void)synchronous;
#endif
}

} // namespace Vkm::GL

