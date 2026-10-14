#include "gl_vertex_array.h"

#include <utility>

#include "gl_error_handle.h"
#include "l_assert.h"

namespace Vkm::GL {

VertexArray::VertexArray() : GLObject(0, GL_VERTEX_ARRAY, 0) {
    VKM_GL_CHECK(glCreateVertexArrays(1, &m_id));
    VKM_ASSERT(m_id != 0);
}

VertexArray::~VertexArray() {
    release();
}

VertexArray& VertexArray::operator=(VertexArray && other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
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

void VertexArray::addBuffer(
    const VertexBuffer& vertexBuffer,
    const VertexBufferLayout& layout,
    uint32_t startIndex
) {
    // The buffer takes the binding numbered as its first attribute, so two buffers fed at their
    // own start indices never share one.
    const uint32_t binding = startIndex;
    const auto stride = static_cast<GLsizei>(layout.getStride());
    VKM_GL_CHECK(glVertexArrayVertexBuffer(m_id, binding, vertexBuffer.getID(), 0, stride));

    const std::vector<VertexBufferElement>& elements = layout.getElements();
    uint32_t offset = 0;

    for (uint32_t i = 0; i < elements.size(); ++i) {
        const VertexBufferElement& element = elements[i];
        const uint32_t index = startIndex + i;

        VKM_GL_CHECK(glEnableVertexArrayAttrib(m_id, index));

        // An integer attribute stays integral in the shader only when it is
        // not normalized; a normalized one is read as a float, which is the
        // same path every non-integer type takes.
        const bool integral = element.type == GL_INT   || element.type == GL_UNSIGNED_INT   ||
                              element.type == GL_BYTE  || element.type == GL_UNSIGNED_BYTE  ||
                              element.type == GL_SHORT || element.type == GL_UNSIGNED_SHORT;

        if (integral && element.normalized == GL_FALSE) {
            VKM_GL_CHECK(glVertexArrayAttribIFormat(m_id, index, element.count, element.type, offset));
        } else {
            const GLboolean normalized = element.normalized;
            const GLenum    type       = element.type;
            VKM_GL_CHECK(glVertexArrayAttribFormat(m_id, index, element.count, type, normalized, offset));
        }
        VKM_GL_CHECK(glVertexArrayAttribBinding(m_id, index, binding));

        offset += element.count * VertexBufferElement::getSizeOfType(element.type);
    }
}

void VertexArray::setDivisor(uint32_t startIndex, uint32_t divisor) {
    VKM_GL_CHECK(glVertexArrayBindingDivisor(m_id, startIndex, divisor));
}

void VertexArray::drawArrays(GLenum mode, int32_t first, int32_t count) const {
    VKM_GL_CHECK(glDrawArrays(mode, first, count));
}

} // namespace Vkm::GL
