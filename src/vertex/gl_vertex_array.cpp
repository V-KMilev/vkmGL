#include "gl_vertex_array.h"

#include <cstdint>
#include <utility>

#include "gl_buffer.h"
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

namespace {

// How a layout element reaches the shader: an integer type keeps its value unless the element is
// normalized; every other type is read as a float.
AttributeKind kindOf(const VertexBufferElement& element) {
    const bool integral = element.type == GL_INT   || element.type == GL_UNSIGNED_INT   ||
                          element.type == GL_BYTE  || element.type == GL_UNSIGNED_BYTE  ||
                          element.type == GL_SHORT || element.type == GL_UNSIGNED_SHORT;
    if (!integral) return AttributeKind::Float;
    return element.normalized == GL_TRUE ? AttributeKind::Normalized : AttributeKind::Integer;
}

} // namespace

void VertexArray::addBuffer(
    const VertexBuffer& vertexBuffer,
    const VertexBufferLayout& layout,
    uint32_t startIndex
) {
    // The buffer takes the binding numbered as its first attribute, so two buffers fed at their
    // own start indices never share one.
    const uint32_t binding = startIndex;
    setVertexBuffer(binding, vertexBuffer, 0, layout.getStride());

    const std::vector<VertexBufferElement>& elements = layout.getElements();
    uint32_t offset = 0;

    for (uint32_t i = 0; i < elements.size(); ++i) {
        const VertexBufferElement& element = elements[i];
        const auto count = static_cast<int32_t>(element.count);
        setAttribute(startIndex + i, binding, count, element.type, offset, kindOf(element));
        offset += element.count * VertexBufferElement::getSizeOfType(element.type);
    }
}

void VertexArray::setDivisor(uint32_t startIndex, uint32_t divisor) {
    VKM_GL_CHECK(glVertexArrayBindingDivisor(m_id, startIndex, divisor));
}

void VertexArray::drawArrays(GLenum mode, int32_t first, int32_t count) const {
    VKM_GL_CHECK(glDrawArrays(mode, first, count));
}

void VertexArray::setAttribute(
    uint32_t index,
    uint32_t binding,
    int32_t count,
    GLenum type,
    uint32_t offset,
    AttributeKind kind
) {
    VKM_GL_CHECK(glEnableVertexArrayAttrib(m_id, index));
    switch (kind) {
        case AttributeKind::Float:
            VKM_GL_CHECK(glVertexArrayAttribFormat(m_id, index, count, type, GL_FALSE, offset));
            break;
        case AttributeKind::Normalized:
            VKM_GL_CHECK(glVertexArrayAttribFormat(m_id, index, count, type, GL_TRUE, offset));
            break;
        case AttributeKind::Integer:
            VKM_GL_CHECK(glVertexArrayAttribIFormat(m_id, index, count, type, offset));
            break;
    }
    VKM_GL_CHECK(glVertexArrayAttribBinding(m_id, index, binding));
}

void VertexArray::setVertexBuffer(
    uint32_t binding,
    const GLBuffer& buffer,
    uint32_t offset,
    uint32_t stride
) {
    setVertexBuffer(binding, BufferSpan{buffer.getID(), offset, buffer.getSize()}, stride);
}

void VertexArray::setVertexBuffer(uint32_t binding, const BufferSpan& span, uint32_t stride) {
    const auto at   = static_cast<GLintptr>(span.offset);
    const auto step = static_cast<GLsizei>(stride);
    VKM_GL_CHECK(glVertexArrayVertexBuffer(m_id, binding, span.buffer, at, step));
}

void VertexArray::setIndexBuffer(const GLBuffer& buffer) {
    VKM_GL_CHECK(glVertexArrayElementBuffer(m_id, buffer.getID()));
}

void VertexArray::drawElementsBaseVertex(
    GLenum mode,
    uint32_t count,
    uint32_t firstIndex,
    int32_t baseVertex
) const {
    const auto* at    = reinterpret_cast<const void*>(uintptr_t{firstIndex} * sizeof(uint32_t));
    const auto  drawn = static_cast<GLsizei>(count);
    VKM_GL_CHECK(glDrawElementsBaseVertex(mode, drawn, GL_UNSIGNED_INT, at, baseVertex));
}

void VertexArray::multiDrawElementsIndirect(GLenum mode, const BufferSpan& commands, uint32_t count) const {
    VKM_GL_CHECK(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commands.buffer));
    const auto* at = reinterpret_cast<const void*>(uintptr_t{commands.offset});
    VKM_GL_CHECK(glMultiDrawElementsIndirect(mode, GL_UNSIGNED_INT, at, static_cast<GLsizei>(count), 0));
}

void VertexArray::drawArraysInstanced(GLenum mode, int32_t first, int32_t count, int32_t instances) const {
    VKM_GL_CHECK(glDrawArraysInstanced(mode, first, count, instances));
}

void VertexArray::multiDrawArrays(
    GLenum mode,
    const int32_t* firsts,
    const int32_t* counts,
    int32_t draws
) const {
    VKM_GL_CHECK(glMultiDrawArrays(mode, firsts, counts, draws));
}

} // namespace Vkm::GL
