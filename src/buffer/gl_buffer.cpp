#include "gl_buffer.h"

#include <utility>

#include "gl_error_handle.h"
#include "l_assert.h"

namespace Core {

GLBuffer::GLBuffer(
    GLenum target,
    const void* data,
    uint32_t size,
    GLenum usage
) : GLObject(target, GL_BUFFER, 0),
    m_size(size),
    m_usage(usage)
{
    VKM_GL_CHECK(glGenBuffers(1, &m_id));
    VKM_ASSERT(m_id != 0);

    bind();
    VKM_GL_CHECK(glBufferData(m_target, m_size, data, m_usage));
}

GLBuffer::~GLBuffer() {
    release();
}

GLBuffer& GLBuffer::operator=(GLBuffer && other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
        m_size  = other.m_size;
        m_usage = other.m_usage;
    }
    return *this;
}

void GLBuffer::release() noexcept {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteBuffers(1, &m_id));
    m_id = 0;
}

void GLBuffer::bind(GLenum target) const {
    // The override is what lets one buffer serve two roles - storage a compute
    // stage writes and, say, the indirect commands a draw reads. Ignoring it
    // silently bound to the wrong target and left the intended one empty.
    VKM_GL_CHECK(glBindBuffer(target == GL_NONE ? m_target : target, m_id));
}

void GLBuffer::unbind(GLenum target) const {
    VKM_GL_CHECK(glBindBuffer(target == GL_NONE ? m_target : target, 0));
}

uint32_t GLBuffer::getSize() const {
    return m_size;
}

GLenum GLBuffer::getTarget() const {
    return m_target;
}

void GLBuffer::update(const void* data, uint32_t size, uint32_t offset) {
    VKM_ASSERT(offset + size <= m_size);
    bind();
    VKM_GL_CHECK(glBufferSubData(m_target, offset, size, data));
}

void GLBuffer::allocate(uint32_t size, const void* data) {
    bind();
    VKM_GL_CHECK(glBufferData(m_target, size, data, m_usage));
    m_size = size;
}

void GLBuffer::bindBase(uint32_t bindingPoint) const {
    VKM_GL_CHECK(glBindBufferBase(m_target, bindingPoint, m_id));
}

void GLBuffer::bindRange(uint32_t bindingPoint, uint32_t offset, uint32_t size) const {
    VKM_GL_CHECK(glBindBufferRange(m_target, bindingPoint, m_id, offset, size));
}

MappedBuffer::MappedBuffer(const GLBuffer& buffer, void* data)
    : m_buffer(&buffer)
    , m_data(data) {}

MappedBuffer::~MappedBuffer() {
    if (m_data == nullptr) return;
    m_buffer->bind();
    VKM_GL_CHECK(glUnmapBuffer(m_buffer->getTarget()));
}

MappedBuffer::MappedBuffer(MappedBuffer && other) noexcept
    : m_buffer(other.m_buffer)
    , m_data(other.m_data) {
    other.m_data = nullptr;
}

MappedBuffer& MappedBuffer::operator=(MappedBuffer && other) noexcept {
    if (this != &other) {
        m_buffer     = other.m_buffer;
        m_data       = other.m_data;
        other.m_data = nullptr;
    }
    return *this;
}

MappedBuffer GLBuffer::map(GLenum access) {
    bind();
    void* data = nullptr;
    VKM_GL_CHECK(data = glMapBuffer(m_target, access));
    return MappedBuffer(*this, data);
}

MappedBuffer GLBuffer::mapRange(uint32_t offset, uint32_t length, GLbitfield access) {
    VKM_ASSERT(offset + length <= m_size);
    bind();
    void* data = nullptr;
    VKM_GL_CHECK(data = glMapBufferRange(m_target, offset, length, access));
    return MappedBuffer(*this, data);
}

} // namespace Core
