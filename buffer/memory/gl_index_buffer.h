#pragma once

#include "gl_buffer.h"

namespace Core {

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

    private:
        uint32_t m_count;
        GLenum m_type;
};
};
