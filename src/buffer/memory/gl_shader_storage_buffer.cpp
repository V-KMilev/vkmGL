#include "gl_shader_storage_buffer.h"

#include <GL/glew.h>

// GL_SHADER_STORAGE_BUFFER requires OpenGL 4.3+
#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif

#include "gl_error_handle.h"

namespace Core {

ShaderStorageBuffer::ShaderStorageBuffer(const void* data, uint32_t size, GLenum usage)
    : GLBuffer(GL_SHADER_STORAGE_BUFFER, data, size, usage) {}

void ShaderStorageBuffer::bindBase(uint32_t bindingPoint) const {
    VKM_GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_id));
}

void ShaderStorageBuffer::bindRange(uint32_t bindingPoint, uint32_t offset, uint32_t size) const {
    VKM_GL_CHECK(glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_id, offset, size));
}
};
