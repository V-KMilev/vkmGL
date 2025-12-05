#include "gl_index_buffer.h"

namespace Core {

namespace {
    uint32_t getTypeSize(GLenum type) {
        switch (type) {
            case GL_UNSIGNED_INT:   return sizeof(GLuint);
            case GL_UNSIGNED_SHORT: return sizeof(GLushort);
            case GL_UNSIGNED_BYTE:  return sizeof(GLubyte);
            default:                return sizeof(GLuint);
        }
    }
}

IndexBuffer::IndexBuffer(
    const void* data,
    uint32_t count,
    GLenum type,
    GLenum usage
) : GLBuffer(GL_ELEMENT_ARRAY_BUFFER, data, count * getTypeSize(type), usage),
    m_count(count),
    m_type(type)
{}

uint32_t IndexBuffer::getCount() const {
    return m_count;
}

GLenum IndexBuffer::getType() const {
    return m_type;
}
};
