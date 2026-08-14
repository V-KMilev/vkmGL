#include "gl_vertex_array.h"

#include <utility>

#include "gl_error_handle.h"
#include "l_assert.h"

namespace Core {

VertexArray::VertexArray() : GLObject(0, GL_VERTEX_ARRAY, 0), m_attributeIndex(0) {
    VKM_GL_CHECK(glGenVertexArrays(1, &m_id));
    VKM_ASSERT(m_id != 0);
}

VertexArray::~VertexArray() {
    release();
}

VertexArray& VertexArray::operator=(VertexArray && other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
        m_attributeIndex = other.m_attributeIndex;
    }
    return *this;
}

void VertexArray::release() noexcept {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteVertexArrays(1, &m_id));
    m_id = 0;
}

void VertexArray::bind() const {
    VKM_GL_CHECK(glBindVertexArray(m_id));
}

void VertexArray::unbind() const {
    VKM_GL_CHECK(glBindVertexArray(0));
}

void VertexArray::addBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout) {
    addBuffer(vertexBuffer, layout, m_attributeIndex);
}

void VertexArray::addBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout, uint32_t startIndex) {
    bind();
    vertexBuffer.bind();

    const std::vector<VertexBufferElement>& elements = layout.getElements();
    uintptr_t offset = 0;

    for (uint32_t i = 0; i < elements.size(); ++i) {
        const VertexBufferElement& element = elements[i];
        uint32_t index = startIndex + i;

        VKM_GL_CHECK(glEnableVertexAttribArray(index));

        // An integer attribute stays integral in the shader only when it is
        // not normalized; a normalized one is read as a float, which is the
        // same path every non-integer type takes.
        const bool integral = element.type == GL_INT   || element.type == GL_UNSIGNED_INT   ||
                              element.type == GL_BYTE  || element.type == GL_UNSIGNED_BYTE  ||
                              element.type == GL_SHORT || element.type == GL_UNSIGNED_SHORT;

        if (integral && element.normalized == GL_FALSE) {
            VKM_GL_CHECK(glVertexAttribIPointer(
                index,
                element.count,
                element.type,
                layout.getStride(),
                reinterpret_cast<const void*>(offset)
            ));
        } else {
            VKM_GL_CHECK(glVertexAttribPointer(
                index,
                element.count,
                element.type,
                element.normalized,
                layout.getStride(),
                reinterpret_cast<const void*>(offset)
            ));
        }

        offset += element.count * VertexBufferElement::getSizeOfType(element.type);
    }

    m_attributeIndex = startIndex + static_cast<uint32_t>(elements.size());
}

void VertexArray::setAttributeDivisor(uint32_t index, uint32_t divisor) {
    bind();
    VKM_GL_CHECK(glVertexAttribDivisor(index, divisor));
}

void VertexArray::drawArrays(GLenum mode, int32_t first, int32_t count) const {
    VKM_GL_CHECK(glDrawArrays(mode, first, count));
}

} // namespace Core
