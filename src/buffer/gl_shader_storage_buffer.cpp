#include "gl_shader_storage_buffer.h"

namespace Core {

ShaderStorageBuffer::ShaderStorageBuffer(const void* data, uint32_t size, GLenum usage)
    : GLBuffer(GL_SHADER_STORAGE_BUFFER, data, size, usage) {}

} // namespace Core
