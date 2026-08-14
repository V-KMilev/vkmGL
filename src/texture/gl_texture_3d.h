#pragma once

#include <cstdint>
#include <string>
#include <GL/glew.h>

#include "gl_object.h"
#include "gl_texture.h"  // TextureWrap / TextureMinFilter / TextureMagFilter

namespace Core {

/**
 * @brief Parameters for a 3D texture (e.g. a froxel volume or an SH probe grid).
 */
struct Texture3DParams {
    uint32_t width  = 0;                                    ///< Texture width
    uint32_t height = 0;                                    ///< Texture height
    uint32_t depth  = 0;                                    ///< Texture depth (slices)

    GLenum internalFormat = GL_RGBA16F;                     ///< Internal format (must be image-load/store-able)
    GLenum format         = GL_RGBA;                        ///< Data format (for optional uploads)
    GLenum type           = GL_FLOAT;                       ///< Data type

    TextureWrap      wrap      = TextureWrap::ClampToEdge;  ///< Wrap on all three axes
    TextureMinFilter minFilter = TextureMinFilter::Linear;  ///< Minifying filter
    TextureMagFilter magFilter = TextureMagFilter::Linear;  ///< Magnification filter
};

/**
 * @brief A 3D OpenGL texture, sampleable and bindable as a compute image.
 *
 * Storage is allocated at construction (contents undefined until written). A
 * compute shader writes it via bindImage(); graphics shaders sample it via
 * bindSlot(). Used for froxel volumetric data and volume textures generally.
 */
class Texture3D : public GLObject {
    public:
        Texture3D() = delete;
        ~Texture3D() override;

        Texture3D(const Texture3D& other) = delete;
        Texture3D& operator=(const Texture3D& other) = delete;

        Texture3D(Texture3D && other) noexcept = default;
        Texture3D& operator=(Texture3D && other) noexcept;

        explicit Texture3D(const std::string& name, const Texture3DParams& params);

    public:
        void bind(GLenum target = GL_NONE) const override;
        void unbind(GLenum target = GL_NONE) const override;

        /**
         * @brief Bind for sampling on texture unit @p slot.
         */
        void bindSlot(uint32_t slot) const;

        /**
         * @brief Bind the whole volume as a compute image on @p unit.
         *
         * @param unit   Image unit (matches `layout(binding = unit) uniform image3D`).
         * @param access GL_READ_ONLY / GL_WRITE_ONLY / GL_READ_WRITE.
         * @param level  Mip level (default 0).
         */
        void bindImage(uint32_t unit, GLenum access, int32_t level = 0) const;

        uint32_t getWidth()  const { return m_params.width; }
        uint32_t getHeight() const { return m_params.height; }
        uint32_t getDepth()  const { return m_params.depth; }

    private:
        void release() noexcept;


    private:
        std::string     m_name;
        Texture3DParams m_params;
};

} // namespace Core
