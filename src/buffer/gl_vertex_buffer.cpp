#include "gl_vertex_buffer.h"

namespace VkmGL {

VertexBuffer::VertexBuffer(
    const void* data,
    uint32_t size,
    GLenum usage
) : GLBuffer(GL_ARRAY_BUFFER, data, size, usage) {}

} // namespace VkmGL
