#pragma once

#include "gl_buffer.h"

namespace Vkm::GL {

/**
 * @brief Uniform buffer (UBO) wrapper.
 *
 * Adds nothing to GLBuffer but its target. bindBase() and bindRange() are
 * inherited and bind as GL_UNIFORM_BUFFER because that is what the buffer was
 * created with, so the type exists to make a call site read as a UBO and to
 * stop one being constructed against the wrong target by accident.
 */
class UniformBuffer : public GLBuffer {
    public:
        UniformBuffer(
            const void* data,
            uint32_t size,
            GLenum usage = GL_STATIC_DRAW
        );
};

} // namespace Vkm::GL
