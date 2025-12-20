#include "gl_buffer.h"

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace Core {

std::unordered_map<GLenum, uint32_t> GLBuffer::s_boundBuffers;

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
    if (m_id == 0) {
        LOG_FATAL("Attempting to delete invalid buffer [ID:%u]", m_id);
        VKM_ASSERT(false);
    }

    VKM_GL_CHECK(glDeleteBuffers(1, &m_id));
}

void GLBuffer::bind(GLenum target) const {
    const auto it = s_boundBuffers.find(m_target);
    if (it == s_boundBuffers.end() || it->second != m_id) {
        VKM_GL_CHECK(glBindBuffer(m_target, m_id));
        s_boundBuffers[m_target] = m_id;
    }
}

void GLBuffer::unbind(GLenum target) const {
    const auto it = s_boundBuffers.find(m_target);
    if (it != s_boundBuffers.end() && it->second == m_id) {
        VKM_GL_CHECK(glBindBuffer(m_target, 0));
        s_boundBuffers[m_target] = 0;
    }
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
