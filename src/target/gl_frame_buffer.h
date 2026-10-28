#pragma once

#include <cstddef>
#include <cstdint>

#include "gl_object.h"

namespace Vkm::GL {

/**
 * @brief OpenGL Frame Buffer Object (FBO) abstraction.
 *
 * Manages FBO creation, deletion, attachment, and status checks, all through direct state
 * access: only bind() changes what is drawn into.
 */
class FrameBuffer : public GLObject {
    public:
        FrameBuffer();
        ~FrameBuffer() override;

        FrameBuffer(const FrameBuffer& other) = delete;
        FrameBuffer& operator=(const FrameBuffer& other) = delete;

        FrameBuffer(FrameBuffer && other) noexcept = default;
        FrameBuffer& operator=(FrameBuffer && other) noexcept;

    public:
        /**
         * @brief Bind the framebuffer object.
         * @param target Unused (default GL_NONE).
         */
        void bind(GLenum target = GL_NONE) const;

        /**
         * @brief Unbind the framebuffer object.
         * @param target Unused (default GL_NONE).
         */
        void unbind(GLenum target = GL_NONE) const;

        /**
         * @brief Bind the default framebuffer (the window backbuffer, FBO 0).
         *
         * The default framebuffer is not a GL object, so it has no instance to
         * bind through - this static stands in so callers never touch raw GL.
         */
        static void bindDefault();

        /**
         * @brief Attach a texture level: a 2D texture, multisample or not, or a whole layered one.
         * @param attachment Framebuffer attachment (e.g., GL_COLOR_ATTACHMENT0).
         * @param textureID  GL texture object ID; 0 detaches.
         * @param level      Mipmap level (default 0).
         */
        void attachTexture(GLenum attachment, uint32_t textureID, int32_t level = 0);

        /**
         * @brief Attach one layer of a texture level: an array's layer, a cube's face, a cube
         *        array's layer * 6 + face.
         * @param attachment Framebuffer attachment (e.g., GL_DEPTH_ATTACHMENT).
         * @param textureID  GL texture object ID.
         * @param level      Mipmap level.
         * @param layer      The layer.
         */
        void attachLayer(GLenum attachment, uint32_t textureID, int32_t level, int32_t layer);

        /**
         * @brief Attach a render buffer to the framebuffer.
         * @param attachment Framebuffer attachment point (e.g., GL_DEPTH_ATTACHMENT).
         * @param renderBufferID GL renderbuffer object ID; 0 detaches.
         */
        void attachRenderBuffer(GLenum attachment, uint32_t renderBufferID);

        /**
         * @brief Set the list of draw color attachments for the framebuffer.
         * @param attachments Array of GL enums indicating color attachments.
         * @param count Number of draw buffers.
         */
        void setDrawBuffers(const GLenum* attachments, int32_t count);

        /**
         * @brief Set a single draw buffer for drawing.
         * @param attachment GL enum for the draw buffer.
         */
        void setDrawBuffer(GLenum attachment);

        /**
         * @brief Set the read buffer for pixel reads.
         * @param attachment GL enum for the read buffer.
         */
        void setReadBuffer(GLenum attachment);

        /**
         * @brief Clear one color draw buffer to @p rgba, whatever is bound.
         *
         * @param drawBuffer The draw buffer's index.
         * @param rgba       Four values, for a float or normalized buffer.
         */
        void clearColor(int32_t drawBuffer, const float rgba[4]) const;

        /**
         * @brief Clear one unsigned-integer color draw buffer to @p rgba, whatever is bound.
         *
         * @param drawBuffer The draw buffer's index.
         * @param rgba       Four values.
         */
        void clearColor(int32_t drawBuffer, const uint32_t rgba[4]) const;

        /**
         * @brief Clear the depth attachment to @p depth, whatever is bound.
         *
         * Depth writes must be on (Context::setDepthWrite): a clear honours the mask.
         *
         * @param depth The value cleared to.
         */
        void clearDepth(float depth) const;

        /**
         * @brief Read a rectangle of the default framebuffer's read buffer into @p pixels.
         *
         * The read binding is put back after.
         *
         * @param x      Left, in pixels from the window's lower left.
         * @param y      Bottom.
         * @param width  Width in pixels.
         * @param height Height in pixels.
         * @param format GL format of what is read, e.g. GL_RGB.
         * @param type   GL type of each channel, e.g. GL_UNSIGNED_BYTE.
         * @param bytes  The room @p pixels has.
         * @param pixels Where the rows land, bottom first, at the pack alignment.
         */
        static void readDefault(
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height,
            GLenum format,
            GLenum type,
            size_t bytes,
            void* pixels
        );

        /**
         * @brief Check if the framebuffer is complete and ready for rendering.
         * @return True if complete; false otherwise.
         */
        bool isComplete() const;

        /**
         * @brief Perform an OpenGL status check for the framebuffer.
         * @return The GL status code from glCheckNamedFramebufferStatus.
         */
        GLenum checkStatus() const;

        /**
         * @brief Blit (copy) a rectangle of pixels from one framebuffer to another.
         * @param sourceID Framebuffer read from; 0 is the default one.
         * @param targetID Framebuffer written to; 0 is the default one.
         * @param srcX0 Source rectangle X0
         * @param srcY0 Source rectangle Y0
         * @param srcX1 Source rectangle X1
         * @param srcY1 Source rectangle Y1
         * @param dstX0 Dest rectangle X0
         * @param dstY0 Dest rectangle Y0
         * @param dstX1 Dest rectangle X1
         * @param dstY1 Dest rectangle Y1
         * @param mask Bitwise OR of buffers to copy (GL_COLOR_BUFFER_BIT, etc)
         * @param filter Interpolation method (GL_NEAREST or GL_LINEAR)
         */
        static void blit(
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
        );

    private:
        /**
         * @brief Delete the FBO and zero m_id.
         *
         * Idempotent, so it is safe on a moved-from framebuffer or after a
         * previous release().
         */
        void release() noexcept;
};

} // namespace Vkm::GL
