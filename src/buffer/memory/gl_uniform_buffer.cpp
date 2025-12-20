#include "gl_uniform_buffer.h"

#include "gl_error_handle.h"

namespace Core {

UniformBuffer::UniformBuffer(const void* data, uint32_t size, GLenum usage)
    : GLBuffer(GL_UNIFORM_BUFFER, data, size, usage) {}

void UniformBuffer::bindBase(uint32_t bindingPoint) const {
    // Encode binding point into key: use GL_UNIFORM_BUFFER as base, encode binding point in upper bits
    // This allows us to reuse GLBuffer's static member for UBO binding point tracking
    // Key format: (GL_UNIFORM_BUFFER << 16) | bindingPoint
    const GLenum compositeKey = static_cast<GLenum>((GL_UNIFORM_BUFFER << 16) | bindingPoint);
    const auto it = s_boundBuffers.find(compositeKey);
    if (it == s_boundBuffers.end() || it->second != m_id) {
        VKM_GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_id));
        s_boundBuffers[compositeKey] = m_id;
    }
}

void UniformBuffer::bindRange(uint32_t bindingPoint, uint32_t offset, uint32_t size) const {
    VKM_GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, m_id, offset, size));
}

};

