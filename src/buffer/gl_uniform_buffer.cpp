#include "gl_uniform_buffer.h"

namespace Vkm::GL {

UniformBuffer::UniformBuffer(const void* data, uint32_t size, GLenum usage)
    : GLBuffer(GL_UNIFORM_BUFFER, data, size, usage) {}

} // namespace Vkm::GL
