#pragma once

#include "gl_object.h"

namespace Core {

/**
 * @brief Encapsulates an OpenGL renderbuffer object.
 *
 * RenderBuffer provides functionality for creation, binding, storage allocation,
 * and querying of OpenGL renderbuffer objects. Renderbuffers are frequently used
 * as attachments for framebuffers and can support multisampling.
 */
class RenderBuffer : public GLObject {
    public:
        RenderBuffer();
        ~RenderBuffer() override;

        RenderBuffer(const RenderBuffer& other) = delete;
        RenderBuffer& operator=(const RenderBuffer& other) = delete;

        RenderBuffer(RenderBuffer && other) noexcept = default;
        RenderBuffer& operator=(RenderBuffer && other) noexcept;

    public:
        /**
         * @brief Binds this renderbuffer to GL_RENDERBUFFER.
         *               glBindRenderbuffer accepts. Present for GLObject.
         */
        void bind() const;

        /**
         * @brief Unbinds this renderbuffer from GL_RENDERBUFFER.
         */
        void unbind() const;

        /**
         * @brief Gets the width of this renderbuffer, as set on the last storage allocation.
         * @return Width in pixels.
         */
        int32_t getWidth() const;

        /**
         * @brief Gets the height of this renderbuffer, as set on the last storage allocation.
         * @return Height in pixels.
         */
        int32_t getHeight() const;

        /**
         * @brief Allocates storage for the renderbuffer in single-sample (non-multisample) mode.
         * @param internalFormat Internal format for the renderbuffer (e.g., GL_DEPTH24_STENCIL8).
         * @param width Width in pixels.
         * @param height Height in pixels.
         */
        void storage(GLenum internalFormat, int32_t width, int32_t height);

        /**
         * @brief Allocates multisample storage for the renderbuffer.
         * @param samples Number of samples for multisampling (anti-aliasing).
         * @param internalFormat Internal format for the renderbuffer.
         * @param width Width in pixels.
         * @param height Height in pixels.
         */
        void storageMultisample(int32_t samples, GLenum internalFormat, int32_t width, int32_t height);

    private:
        /**
         * @brief Delete the renderbuffer and zero m_id.
         *
         * Idempotent, so it is safe on a moved-from renderbuffer or after a previous release().
         */
        void release() noexcept;

    private:
        int32_t m_width;
        int32_t m_height;
};

} // namespace Core
