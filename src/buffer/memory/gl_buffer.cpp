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

GLBuffer& GLBuffer::operator=(GLBuffer&& other) noexcept {
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

void GLBuffer::update(const void* data, uint32_t size, uint32_t offset) {
    bind();
    VKM_GL_CHECK(glBufferSubData(m_target, offset, size, data));
}

void* GLBuffer::map(GLenum access) {
    bind();
    return glMapBuffer(m_target, access);
}

void* GLBuffer::mapRange(uint32_t offset, uint32_t length, GLbitfield access) {
    bind();
    return glMapBufferRange(m_target, offset, length, access);
}

void GLBuffer::unmap() {
    bind();
    VKM_GL_CHECK(glUnmapBuffer(m_target));
}

};
