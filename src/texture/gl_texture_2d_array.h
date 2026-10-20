#pragma once

#include <cstdint>
#include <string>

#include <GL/glew.h>

#include "gl_object.h"
#include "gl_texture.h"  // TextureWrap / TextureMinFilter / TextureMagFilter

namespace Vkm::GL {

/**
 * @brief Parameters for a 2D array texture: layers of one size and format, sampled as one.
 */
struct Texture2DArrayParams {
    uint32_t width  = 0;                                    ///< Each layer's width
    uint32_t height = 0;                                    ///< Each layer's height
    uint32_t layers = 1;                                    ///< How many layers
    uint32_t levels = 1;                                    ///< Mip levels each layer holds

    GLenum internalFormat = GL_RGBA8;                       ///< Sized format of the storage

    TextureWrap      wrapS     = TextureWrap::ClampToEdge;  ///< S axis wrapping
    TextureWrap      wrapT     = TextureWrap::ClampToEdge;  ///< T axis wrapping
    TextureMinFilter minFilter = TextureMinFilter::Linear;  ///< Minifying filter
    TextureMagFilter magFilter = TextureMagFilter::Linear;  ///< Magnification filter

    /// Anisotropic degree (1 = off); clamped to the driver's limit.
    float maxAnisotropy = 1.0f;

    /// Sampled through a view in this format - an sRGB read of an RGBA8 storage a shader
    /// writes - rather than the storage's own; GL_NONE samples the storage.
    GLenum viewFormat = GL_NONE;
};

/**
 * @brief A 2D array OpenGL texture with immutable storage: layers a shader samples by index, writes
 *        as images, or a framebuffer draws into a layer at a time.
 *
 * Every edit goes through direct state access. The storage is fixed at creation; a new size or
 * layer count is a new Texture2DArray.
 */
class Texture2DArray : public GLObject {
    public:
        Texture2DArray() = delete;
        ~Texture2DArray() override;

        Texture2DArray(const Texture2DArray& other) = delete;
        Texture2DArray& operator=(const Texture2DArray& other) = delete;

        Texture2DArray(Texture2DArray && other) noexcept;
        Texture2DArray& operator=(Texture2DArray && other) noexcept;

        explicit Texture2DArray(const std::string& name, const Texture2DArrayParams& params);

    public:
        /**
         * @brief Bind for sampling on texture unit @p slot: the view, when it has one.
         *
         * @param slot Texture unit.
         */
        void bindSlot(uint32_t slot) const;

        /**
         * @brief Bind one level of every layer as an image, in the storage's format.
         *
         * @param unit   Image unit (`layout(binding = unit) uniform image2DArray`).
         * @param access GL_READ_ONLY, GL_WRITE_ONLY or GL_READ_WRITE.
         * @param level  The level.
         */
        void bindImage(uint32_t unit, GLenum access, uint32_t level = 0) const;

        /**
         * @brief Fill one level of every layer with a single value.
         *
         * @param level  The level.
         * @param format GL format of @p value, e.g. GL_RGBA.
         * @param type   GL type of each of its channels.
         * @param value  One texel's worth; null clears to zero.
         */
        void clear(uint32_t level, GLenum format, GLenum type, const void* value) const;

        /// Build every level below 0 of every layer from level 0.
        void generateMipmaps() const;

        /// The parameters it was created with.
        const Texture2DArrayParams& getParams() const { return m_params; }

        uint32_t getWidth()  const { return m_params.width; }
        uint32_t getHeight() const { return m_params.height; }
        uint32_t layers()    const { return m_params.layers; }
        uint32_t levels()    const { return m_params.levels; }

    private:
        /// Delete the view and the storage, and zero both names.
        void release() noexcept;

    private:
        std::string          m_name;      ///< Debug label, for GL object naming.
        Texture2DArrayParams m_params;
        uint32_t             m_view = 0;  ///< The sampled view in params.viewFormat; 0 for none.
};

} // namespace Vkm::GL
