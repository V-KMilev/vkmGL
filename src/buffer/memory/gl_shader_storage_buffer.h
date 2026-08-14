#pragma once

#include "gl_buffer.h"

namespace Core {

/**
 * @brief Shader Storage Buffer Object (SSBO) wrapper.
 *
 * Provides creation and binding helpers for SSBOs used by compute/graphics
 * shaders to read/write arbitrary data.
 */
class ShaderStorageBuffer : public GLBuffer {
    public:
        ShaderStorageBuffer(
            const void* data,
            uint32_t size,
            GLenum usage = GL_DYNAMIC_DRAW
        );

    public:
        /**
         * @brief Binds the entire shader storage buffer object to a binding point.
         *
         * This method binds the shader storage buffer to the specified binding point
         * using glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, bufferId).
         * This allows shaders to access the buffer's complete range.
         *
         * @param bindingPoint The index of the SSBO binding point.
         */
        void bindBase(uint32_t bindingPoint) const;

        /**
         * @brief Binds a specific range of the shader storage buffer to a binding point.
         *
         * Binds part of the SSBO, starting at the specified byte offset and spanning
         * the indicated number of bytes, to the given binding point using glBindBufferRange.
         * Shaders will only see the specified range.
         *
         * @param bindingPoint The SSBO binding point index.
         * @param offset The byte offset within the buffer to start binding from.
         * @param size The number of bytes to bind starting from offset.
         */
        void bindRange(uint32_t bindingPoint, uint32_t offset, uint32_t size) const;
};

} // namespace Core
