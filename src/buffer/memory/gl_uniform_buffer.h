#pragma once

#include "gl_buffer.h"

namespace Core {

/**
 * @brief Uniform buffer (UBO) wrapper.
 *
 * Provides creation and binding helpers for uniform buffer objects, including
 * binding to specific binding points or ranges.
 */
class UniformBuffer : public GLBuffer {
    public:
        UniformBuffer(
            const void* data,
            uint32_t size,
            GLenum usage = GL_STATIC_DRAW
        );

    public:
        /**
         * @brief Binds the entire uniform buffer object to a binding point.
         *
         * This method binds the uniform buffer to the specified binding point so shaders can access it. 
         * It calls glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, bufferId).
         *
         * @param bindingPoint The index of the UBO binding point.
         */
        void bindBase(uint32_t bindingPoint) const;

        /**
         * @brief Binds a specific range of the uniform buffer to a binding point.
         *
         * This method binds a range of the UBO (from offset, for size bytes) to a binding point using glBindBufferRange.
         *
         * @param bindingPoint The UBO binding point index.
         * @param offset The byte offset into the buffer to start the binding range.
         * @param size The number of bytes to bind starting from offset.
         */
        void bindRange(uint32_t bindingPoint, uint32_t offset, uint32_t size) const;
};

} // namespace Core
