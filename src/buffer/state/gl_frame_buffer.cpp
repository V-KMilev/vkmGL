#include "gl_frame_buffer.h"

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace Core {

FrameBuffer::FrameBuffer() : GLObject(GL_FRAMEBUFFER, GL_FRAMEBUFFER, 0) {
    VKM_GL_CHECK(glGenFramebuffers(1, &m_id));
    VKM_ASSERT(m_id != 0);
}

FrameBuffer::~FrameBuffer() {
    release();
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept {
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

void FrameBuffer::attachTexture(FrameBufferDimension dim, const FrameBufferAttachment& attachment) {
    bind();

    switch (dim) {
        case FrameBufferDimension::ND:
            VKM_GL_CHECK(glFramebufferTexture(
                GL_FRAMEBUFFER,
                attachment.attachment,
                attachment.textureID,
                attachment.level
            ));
            break;

        case FrameBufferDimension::D1:
            VKM_GL_CHECK(glFramebufferTexture1D(
                GL_FRAMEBUFFER,
                attachment.attachment,
                attachment.textarget,
                attachment.textureID,
                attachment.level
            ));
            break;

        case FrameBufferDimension::D2:
            VKM_GL_CHECK(glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                attachment.attachment,
                attachment.textarget,
                attachment.textureID,
                attachment.level
            ));
            break;

        case FrameBufferDimension::D3:
            VKM_GL_CHECK(glFramebufferTexture3D(
                GL_FRAMEBUFFER,
                attachment.attachment,
                attachment.textarget,
                attachment.textureID,
                attachment.level,
                attachment.layer
            ));
            break;
    }
}

void FrameBuffer::attachTexture2D(GLenum attachment, uint32_t textureID, int32_t level) {
    attachTexture2D(attachment, GL_TEXTURE_2D, textureID, level);
}

void FrameBuffer::attachTexture2D(GLenum attachment, GLenum textarget, uint32_t textureID, int32_t level) {
    bind();
    VKM_GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textarget, textureID, level));
}

void FrameBuffer::attachRenderBuffer(GLenum attachment, uint32_t renderBufferID) {
    bind();
    VKM_GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBufferID));
}

void FrameBuffer::setDrawBuffers(const GLenum* attachments, int32_t count) {
    bind();
    VKM_GL_CHECK(glDrawBuffers(count, attachments));
}

void FrameBuffer::setDrawBuffer(GLenum attachment) {
    bind();
    VKM_GL_CHECK(glDrawBuffer(attachment));
}

void FrameBuffer::setReadBuffer(GLenum attachment) {
    bind();
    VKM_GL_CHECK(glReadBuffer(attachment));
}

bool FrameBuffer::isComplete() const {
    return checkStatus() == GL_FRAMEBUFFER_COMPLETE;
}

GLenum FrameBuffer::checkStatus() const {
    bind();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

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
    int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1,
    int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1,
    GLbitfield mask, GLenum filter
) {
    VKM_GL_CHECK(glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter));
}
};
