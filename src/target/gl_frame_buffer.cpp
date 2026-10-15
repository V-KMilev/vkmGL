#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "BUFFER"

#include "gl_frame_buffer.h"

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace Vkm::GL {

FrameBuffer::FrameBuffer() : GLObject(GL_FRAMEBUFFER, GL_FRAMEBUFFER, 0) {
    VKM_GL_CHECK(glCreateFramebuffers(1, &m_id));
    VKM_ASSERT(m_id != 0);
}

FrameBuffer::~FrameBuffer() {
    release();
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer && other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
    }
    return *this;
}

void FrameBuffer::release() noexcept {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteFramebuffers(1, &m_id));
    m_id = 0;
}

void FrameBuffer::bind(GLenum target) const {
    VKM_GL_CHECK(glBindFramebuffer(target == GL_NONE ? GL_FRAMEBUFFER : target, m_id));
}

void FrameBuffer::unbind(GLenum target) const {
    VKM_GL_CHECK(glBindFramebuffer(target == GL_NONE ? GL_FRAMEBUFFER : target, 0));
}

void FrameBuffer::bindDefault() {
    VKM_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::attachTexture(GLenum attachment, uint32_t textureID, int32_t level) {
    VKM_GL_CHECK(glNamedFramebufferTexture(m_id, attachment, textureID, level));
}

void FrameBuffer::attachLayer(GLenum attachment, uint32_t textureID, int32_t level, int32_t layer) {
    VKM_GL_CHECK(glNamedFramebufferTextureLayer(m_id, attachment, textureID, level, layer));
}

void FrameBuffer::attachRenderBuffer(GLenum attachment, uint32_t renderBufferID) {
    VKM_GL_CHECK(glNamedFramebufferRenderbuffer(m_id, attachment, GL_RENDERBUFFER, renderBufferID));
}

void FrameBuffer::setDrawBuffers(const GLenum* attachments, int32_t count) {
    VKM_GL_CHECK(glNamedFramebufferDrawBuffers(m_id, count, attachments));
}

void FrameBuffer::setDrawBuffer(GLenum attachment) {
    VKM_GL_CHECK(glNamedFramebufferDrawBuffer(m_id, attachment));
}

void FrameBuffer::setReadBuffer(GLenum attachment) {
    VKM_GL_CHECK(glNamedFramebufferReadBuffer(m_id, attachment));
}

bool FrameBuffer::isComplete() const {
    return checkStatus() == GL_FRAMEBUFFER_COMPLETE;
}

GLenum FrameBuffer::checkStatus() const {
    const GLenum status = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        const char* errorStr = "Unknown error";
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:                     errorStr = "UNDEFINED"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         errorStr = "INCOMPLETE_ATTACHMENT"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorStr = "MISSING_ATTACHMENT"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        errorStr = "INCOMPLETE_DRAW_BUFFER"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        errorStr = "INCOMPLETE_READ_BUFFER"; break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                   errorStr = "UNSUPPORTED"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        errorStr = "INCOMPLETE_MULTISAMPLE"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      errorStr = "INCOMPLETE_LAYER_TARGETS"; break;
        }
        LOG_ERROR("Framebuffer [ID:%u] incomplete: %s", m_id, errorStr);
    }

    return status;
}

void FrameBuffer::blit(
    uint32_t sourceID,
    uint32_t targetID,
    int32_t srcX0,
    int32_t srcY0,
    int32_t srcX1,
    int32_t srcY1,
    int32_t dstX0,
    int32_t dstY0,
    int32_t dstX1,
    int32_t dstY1,
    GLbitfield mask,
    GLenum filter
) {
    VKM_GL_CHECK(glBlitNamedFramebuffer(
        sourceID,
        targetID,
        srcX0,
        srcY0,
        srcX1,
        srcY1,
        dstX0,
        dstY0,
        dstX1,
        dstY1,
        mask,
        filter
    ));
}

} // namespace Vkm::GL
