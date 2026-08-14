#include "gl_vertex_buffer_layout.h"

namespace Core {

VertexBufferLayout::VertexBufferLayout() : m_stride(0), m_elements({}) {}

uint32_t VertexBufferLayout::getStride() const {
    return m_stride;
}

const std::vector<VertexBufferElement>& VertexBufferLayout::getElements() const {
    return m_elements;
}

void VertexBufferLayout::clear() {
    m_elements.clear();
    m_stride = 0;
}

template<>
void VertexBufferLayout::push<float>(uint32_t count) {
    m_elements.push_back({GL_FLOAT, count, GL_FALSE});
    m_stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
}

template<>
void VertexBufferLayout::push<uint32_t>(uint32_t count) {
    m_elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    m_stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
}

template<>
void VertexBufferLayout::push<int32_t>(uint32_t count) {
    m_elements.push_back({GL_INT, count, GL_FALSE});
    m_stride += count * VertexBufferElement::getSizeOfType(GL_INT);
}

template<>
void VertexBufferLayout::push<uint8_t>(uint32_t count) {
    // Unsigned bytes are typically normalized (0-255 -> 0.0-1.0)
    m_elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
    m_stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
}

} // namespace Core
