#pragma once

#include "gl_buffer.h"

namespace VkmGL {

/**
 * @brief Shader Storage Buffer Object (SSBO) wrapper.
 *
 * Adds nothing to GLBuffer but its target. bindBase() and bindRange() are
 * inherited and bind as GL_SHADER_STORAGE_BUFFER because that is what the
 * buffer was created with, so the type exists to make a call site read as an
 * SSBO and to stop one being constructed against the wrong target by accident.
 */
class ShaderStorageBuffer : public GLBuffer {
    public:
        ShaderStorageBuffer(
            const void* data,
            uint32_t size,
            GLenum usage = GL_DYNAMIC_DRAW
        );
};

} // namespace VkmGL
