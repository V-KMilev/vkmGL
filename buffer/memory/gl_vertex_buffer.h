#pragma once

#include "gl_buffer.h"

namespace Core {

/**
 * @brief Vertex buffer (VBO) wrapping an OpenGL array buffer.
 *
 * Owns a GPU vertex buffer; uploads data on construction and disallows copy/move
 * to keep ownership simple.
 */
class VertexBuffer : public GLBuffer {
    public:
        VertexBuffer(
            const void* data,
            uint32_t size,
            GLenum usage = GL_STATIC_DRAW
        );
};
};
