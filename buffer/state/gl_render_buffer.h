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

        RenderBuffer(RenderBuffer&& other) = delete;
        RenderBuffer& operator=(RenderBuffer&& other) = delete;

    public:
        /**
         * @brief Binds this renderbuffer to GL_RENDERBUFFER or the provided target.
         * @param target The target to bind to. Defaults to GL_NONE (uses internal target).
         */
        void bind(GLenum target = GL_NONE) const override;

        /**
         * @brief Unbinds this renderbuffer from its target.
         * @param target The target to unbind from. Defaults to GL_NONE (uses internal target).
         */
        void unbind(GLenum target = GL_NONE) const override;

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
            int32_t m_width;
            int32_t m_height;
    };
};
