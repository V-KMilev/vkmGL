#include "gl_vertex_array.h"

#include <cassert>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace Core {

VertexArray::VertexArray() : GLObject(0, GL_VERTEX_ARRAY, 0), m_attributeIndex(0) {
    VKM_GL_CHECK(glGenVertexArrays(1, &m_id));
    VKM_ASSERT(m_id != 0);
}

VertexArray::~VertexArray() {
    if (m_id == 0) {
        LOG_FATAL("Attempting to delete invalid vertex array [ID:%u]", m_id);
        assert(false && "Attempting to delete invalid vertex array");
        return;
    }

    VKM_GL_CHECK(glDeleteVertexArrays(1, &m_id));
}

void VertexArray::bind(GLenum target) const {
    VKM_GL_CHECK(glBindVertexArray(m_id));
}

void VertexArray::unbind(GLenum target) const {
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

        // Use glVertexAttribIPointer for integer types (no normalization)
        if (element.type == GL_INT || element.type == GL_UNSIGNED_INT ||
            element.type == GL_BYTE || element.type == GL_UNSIGNED_BYTE ||
            element.type == GL_SHORT || element.type == GL_UNSIGNED_SHORT) {
            if (element.normalized == GL_FALSE) {
                // Integer attributes (passed as-is to shader)
                VKM_GL_CHECK(glVertexAttribIPointer(
                    index,
                    element.count,
                    element.type,
                    layout.getStride(),
                    reinterpret_cast<const void*>(offset)
                ));
            } else {
                // Normalized integer (converted to float 0.0-1.0)
                VKM_GL_CHECK(glVertexAttribPointer(
                    index,
                    element.count,
                    element.type,
                    element.normalized,
                    layout.getStride(),
                    reinterpret_cast<const void*>(offset)
                ));
            }
        } else {
            // Float attributes
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
};
