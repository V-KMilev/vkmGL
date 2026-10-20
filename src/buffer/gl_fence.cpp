#include "gl_fence.h"

#include <utility>

#include "gl_error_handle.h"

namespace Vkm::GL {

Fence::~Fence() {
    release();
}

Fence::Fence(Fence && other) noexcept
    : m_sync(std::exchange(other.m_sync, nullptr)) {}

Fence& Fence::operator=(Fence && other) noexcept {
    if (this != &other) {
        release();
        m_sync = std::exchange(other.m_sync, nullptr);
    }
    return *this;
}

void Fence::release() noexcept {
    if (m_sync == nullptr) return;
    VKM_GL_CHECK(glDeleteSync(m_sync));
    m_sync = nullptr;
}

void Fence::place() {
    release();
    VKM_GL_CHECK(m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
}

bool Fence::passed() {
    if (m_sync == nullptr) return true;
    GLenum status = GL_TIMEOUT_EXPIRED;
    VKM_GL_CHECK(status = glClientWaitSync(m_sync, 0, 0));
    if (status != GL_ALREADY_SIGNALED && status != GL_CONDITION_SATISFIED) return false;
    release();
    return true;
}

void Fence::wait() {
    if (m_sync == nullptr) return;
    // Flushed on the first wait, so the fence reaches the GPU rather than waiting forever.
    GLbitfield flags  = GL_SYNC_FLUSH_COMMANDS_BIT;
    GLenum     status = GL_TIMEOUT_EXPIRED;
    do {
        VKM_GL_CHECK(status = glClientWaitSync(m_sync, flags, 1000000000u));
        flags = 0;
    } while (status == GL_TIMEOUT_EXPIRED);
    release();
}

} // namespace Vkm::GL
