#include "gl_uniform_buffer.h"

#include "gl_error_handle.h"

namespace Core {

UniformBuffer::UniformBuffer(const void* data, uint32_t size, GLenum usage)
    : GLBuffer(GL_UNIFORM_BUFFER, data, size, usage) {}

void UniformBuffer::bindBase(uint32_t bindingPoint) const {
    VKM_GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_id));
}

void UniformBuffer::bindRange(uint32_t bindingPoint, uint32_t offset, uint32_t size) const {
    VKM_GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_id, offset, size));
}
};
