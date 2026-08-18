#pragma once

#include "gl_buffer.h"

namespace Vkm::GL {

/**
 * @brief Index buffer (IBO) wrapping an OpenGL element array buffer.
 *
 * Stores index data for draw calls and tracks count/type used with
 * glDrawElements and related APIs.
 */
class IndexBuffer : public GLBuffer {
    public:
        IndexBuffer(
            const void* data,
            uint32_t count,
            GLenum type = GL_UNSIGNED_INT,
            GLenum usage = GL_STATIC_DRAW
        );

    public:
        /**
         * @brief Get the number of indices in this buffer.
         * @return Index count.
         */
        uint32_t getCount() const;

        /**
         * @brief Get the OpenGL type of each index in this buffer.
         * @return Index type (GL_UNSIGNED_INT, GL_UNSIGNED_SHORT, etc).
         */
        GLenum getType() const;

        /**
         * @brief Draw the currently-bound vertex array as indexed geometry.
         *
         * Issues glDrawElements with this buffer's count + type; the caller binds
         * the vertex array (and this index buffer) first.
         * @param mode Primitive type (default GL_TRIANGLES).
         */
        void draw(GLenum mode = GL_TRIANGLES) const;

        /**
         * @brief Instanced indexed draw with a base-instance offset.
         *
         * @p baseInstance offsets which per-instance attribute element each
         * instance reads (GL 4.2+ / ARB_base_instance), so several runs can share
         * one uploaded instance buffer. The caller binds the vertex array first.
         * @param instanceCount Number of instances to draw.
         * @param baseInstance  First per-instance attribute element to read.
         * @param mode          Primitive type (default GL_TRIANGLES).
         */
        void drawInstanced(uint32_t instanceCount, uint32_t baseInstance, GLenum mode = GL_TRIANGLES) const;

    private:
        uint32_t m_count;
        GLenum m_type;
};

} // namespace Vkm::GL
