#pragma once

#include "gl_object.h"

namespace Core {

/**
* @brief Enum class to specify the dimensionality of a framebuffer attachment.
*/
enum class FrameBufferDimension {
    ND = 0,  ///< Layered/framebuffer texture (glFramebufferTexture)
    D1 = 1,  ///< 1D texture (glFramebufferTexture1D)
    D2 = 2,  ///< 2D texture (glFramebufferTexture2D)
    D3 = 3   ///< 3D texture (glFramebufferTexture3D)
};

/**
* @brief Describes a framebuffer attachment, which can be a texture or renderbuffer.
*
* Holds all information required to attach a texture or renderbuffer to a framebuffer.
* The `attachment` specifies to which framebuffer attachment point (e.g., GL_COLOR_ATTACHMENT0).
* The `textarget` specifies the texture target type (e.g., GL_TEXTURE_2D).
* `textureID` references the GL object, while `level` and `layer` are used for mipmap and array/3D slices.
*/
struct FrameBufferAttachment {
    GLenum attachment = GL_COLOR_ATTACHMENT0; ///< Attachment point (e.g. GL_COLOR_ATTACHMENT0)
    GLenum textarget = GL_TEXTURE_2D;        ///< Texture target (e.g. GL_TEXTURE_2D)
    uint32_t textureID = 0;                  ///< OpenGL texture object ID
    int32_t level = 0;                       ///< Mipmap level
    int32_t layer = 0;                       ///< Layer for array/3D textures

    FrameBufferAttachment() = default;

    /**
    * @brief Create attachment with a specific attachment point and texture.
    * @param attach GL attachment point.
    * @param texID Texture object ID.
    * @param lvl Mipmap level.
    */
    FrameBufferAttachment(
        GLenum attach,
        uint32_t texID,
        int32_t lvl = 0
    ) : attachment(attach),
        textureID(texID),
        level(lvl) {}

    /**
    * @brief Create attachment with detailed texture target/layer control.
    * @param attach GL attachment point.
    * @param target GL texture target.
    * @param texID Texture object ID.
    * @param lvl Mipmap level.
    * @param lyr Layer (for 3D/array textures).
    */
    FrameBufferAttachment(
        GLenum attach,
        GLenum target,
        uint32_t texID,
        int32_t lvl = 0,
        int32_t lyr = 0
    ) : attachment(attach),
        textarget(target),
        textureID(texID),
        level(lvl),
        layer(lyr) {}
};

/**
* @brief OpenGL Frame Buffer Object (FBO) abstraction.
*
* Manages FBO creation, deletion, attachment, and status checks.
*/
class FrameBuffer : public GLObject {
    public:
        FrameBuffer();
        ~FrameBuffer() override;

        FrameBuffer(const FrameBuffer& other) = delete;
        FrameBuffer& operator=(const FrameBuffer& other) = delete;

        FrameBuffer(FrameBuffer&& other) noexcept = default;
        FrameBuffer& operator=(FrameBuffer&& other) noexcept;

    public:
        /**
        * @brief Bind the framebuffer object.
        * @param target Unused (default GL_NONE).
        */
        void bind(GLenum target = GL_NONE) const override;

        /**
        * @brief Unbind the framebuffer object.
        * @param target Unused (default GL_NONE).
        */
        void unbind(GLenum target = GL_NONE) const override;

        /**
        * @brief Attach a texture to the framebuffer using a FrameBufferAttachment.
        * @param dim The dimensionality/type of the texture.
        * @param attachment The attachment data.
        */
        void attachTexture(FrameBufferDimension dim, const FrameBufferAttachment& attachment);

        /**
        * @brief Attach a 2D texture to a framebuffer attachment point.
        * @param attachment Framebuffer attachment (e.g., GL_COLOR_ATTACHMENT0).
        * @param textureID GL texture object ID.
        * @param level Mipmap level (default 0).
        */
        void attachTexture2D(GLenum attachment, uint32_t textureID, int32_t level = 0);

        /**
        * @brief Attach a 2D texture with a specified texture target.
        * @param attachment Framebuffer attachment (e.g., GL_COLOR_ATTACHMENT0).
        * @param textarget Texture target (e.g., GL_TEXTURE_2D).
        * @param textureID GL texture object ID.
        * @param level Mipmap level (default 0).
        */
        void attachTexture2D(GLenum attachment, GLenum textarget, uint32_t textureID, int32_t level = 0);

        /**
        * @brief Attach a render buffer to the framebuffer.
        * @param attachment Framebuffer attachment point (e.g., GL_DEPTH_ATTACHMENT).
        * @param renderBufferID GL renderbuffer object ID.
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
        * @brief Check if the framebuffer is complete and ready for rendering.
        * @return True if complete; false otherwise.
        */
        bool isComplete() const;

        /**
        * @brief Perform an OpenGL status check for the framebuffer.
        * @return The GL status code from glCheckFramebufferStatus.
        */
        GLenum checkStatus() const;

        /**
        * @brief Blit (copy) a rectangle of pixels from one framebuffer to another.
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
            int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1,
            int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1,
            GLbitfield mask, GLenum filter
        );

    private:
        /// Delete the FBO and zero m_id. Idempotent — safe on a moved-from
        /// framebuffer or after a previous release().
        void release() noexcept;
};
};
