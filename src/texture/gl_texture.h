#pragma once

#include <cstdint>
#include <string>
#include <GL/glew.h>

#include "gl_object.h"

namespace Vkm::GL {

/**
 * @brief Enum specifying texture wrapping mode.
 */
enum class TextureWrap {
    Repeat,             ///< Repeat the texture
    MirroredRepeat,     ///< Mirror and repeat the texture
    ClampToEdge,        ///< Clamp texture coordinates to edges
    ClampToBorder       ///< Clamp to user-defined border color
};

/**
 * @brief Enum specifying minifying filter for textures.
 */
enum class TextureMinFilter : int {
    Nearest,                ///< Nearest texel
    Linear,                 ///< Linear interpolation
    NearestMipmapNearest,   ///< Nearest mipmap, nearest texel
    LinearMipmapNearest,    ///< Nearest mipmap, linear texel
    NearestMipmapLinear,    ///< Linear mipmap, nearest texel
    LinearMipmapLinear      ///< Linear mipmap, linear texel
};

/**
 * @brief Enum specifying magnification filter for textures.
 */
enum class TextureMagFilter : int {
    Nearest,    ///< Nearest texel
    Linear      ///< Linear interpolation
};

/**
 * @brief Map a wrap mode to the OpenGL enum glTexParameteri expects.
 *
 * Free functions rather than per-class statics: the mapping depends only on
 * the enum, and every texture type plus Sampler needs the same answer.
 *
 * @param wrap Wrap mode to translate.
 * @return The corresponding GL_* constant.
 */
GLenum toGLenum(TextureWrap wrap);

/**
 * @brief Map a minification filter to its OpenGL enum.
 *
 * @param filter Filter to translate.
 * @return The corresponding GL_* constant.
 */
GLenum toGLenum(TextureMinFilter filter);

/**
 * @brief Map a magnification filter to its OpenGL enum.
 *
 * @param filter Filter to translate.
 * @return The corresponding GL_* constant.
 */
GLenum toGLenum(TextureMagFilter filter);

/**
 * @brief The highest degree of anisotropic filtering this driver will accept.
 *
 * GL_TEXTURE_MAX_ANISOTROPY is only core in 4.6, so on the 4.3 context this
 * library targets it exists only when GL_ARB_texture_filter_anisotropic or its
 * EXT predecessor is present. Returns 1.0 when neither is, which is the value
 * meaning "no anisotropy" and lets callers degrade to plain trilinear without
 * a separate capability test.
 *
 * Queried once on first call, so it needs a current context but costs nothing
 * after that.
 *
 * @return GL_MAX_TEXTURE_MAX_ANISOTROPY, or 1.0 when unsupported.
 */
float maxSupportedAnisotropy();

/**
 * @brief Parameters for initializing a 2D texture.
 */
struct Texture2DParams {
    uint32_t width  = 0;                                                  ///< Texture width
    uint32_t height = 0;                                                  ///< Texture height

    GLenum internalFormat = GL_RGBA8;                                     ///< Internal OpenGL format
    GLenum format         = GL_RGBA;                                      ///< Data format
    GLenum type           = GL_UNSIGNED_BYTE;                             ///< Data type

    TextureWrap wrapS          = TextureWrap::ClampToEdge;                ///< S axis wrapping
    TextureWrap wrapT          = TextureWrap::ClampToEdge;                ///< T axis wrapping
    TextureMinFilter minFilter = TextureMinFilter::LinearMipmapLinear;    ///< Minifying filter
    TextureMagFilter magFilter = TextureMagFilter::Linear;                ///< Magnification filter

    float maxAnisotropy = 1.0f;                                           ///< Anisotropic filtering degree (1 = off); clamped to the driver's limit

    bool generateMipmaps = true;                                          ///< Automatically generate mipmaps
    const void* data     = nullptr;                                       ///< Pointer to initial data
};

/**
 * @brief Params preset for an off-screen render-target texture: clamped, no
 * mipmaps, GL_FLOAT source type, no initial data. Filtering defaults to
 * nearest (depth / data targets); colour targets pass linear.
 */
Texture2DParams renderTargetParams(
    uint32_t width, uint32_t height, GLenum internalFormat, GLenum format,
    TextureMinFilter minFilter = TextureMinFilter::Nearest,
    TextureMagFilter magFilter = TextureMagFilter::Nearest);


/**
 * @brief Represents a 2D OpenGL texture object.
 * 
 * Provides methods for binding, setting parameters, updating data,
 * and loading from file.
 */
class Texture2D : public GLObject {
    public:
        Texture2D() = delete;
        ~Texture2D() override;

        Texture2D(const Texture2D& other) = delete;
        Texture2D& operator=(const Texture2D& other) = delete;

        Texture2D(Texture2D && other) noexcept = default;
        Texture2D& operator=(Texture2D && other) noexcept;

        explicit Texture2D(
            const std::string& name,
            const Texture2DParams& params
        );

        explicit Texture2D(
            const std::string& filePath,
            bool flipVertically = true,
            bool srgb = false
        );

    public:
        /**
         * @brief Bind the texture to the current OpenGL context.
         * @param target Target texture type, defaults to GL_NONE (use internal target).
         */
        void bind(GLenum target = GL_NONE) const;

        /**
         * @brief Unbind the texture from the bind point.
         * @param target Target texture type, defaults to GL_NONE (use internal target).
         */
        void unbind(GLenum target = GL_NONE) const;

        /**
         * @brief Bind the texture to a given texture slot.
         * @param slot   Texture unit slot.
         * @param target Texture type.
         */
        void bindSlot(uint32_t slot, GLenum target = GL_NONE) const;

        /**
         * @brief Get the parameters of the texture.
         * @return The parameters of the texture.
        */
        const Texture2DParams& getParams() const { return m_params; }

        /**
         * @brief Get the name of the texture.
         */
        const std::string& getName() const { return m_name; }

        /**
         * @brief Get the file path associated with this texture, if any.
         */
        const std::string& getPath() const { return m_path; }

        /**
         * @brief Set new data for the texture.
         * @param data   Pointer to pixel data.
         * @param width  Width of new data.
         * @param height Height of new data.
         * @param format OpenGL data format.
         * @param type   OpenGL data type.
         */
        void setData(const void* data, uint32_t width, uint32_t height, GLenum format, GLenum type);

        /**
         * @brief Set the S and T coordinate wrapping modes.
         * @param s S coordinate wrap mode.
         * @param t T coordinate wrap mode.
         */
        void setWrap(TextureWrap s, TextureWrap t);

        /**
         * @brief Set the min/mag filtering for the texture.
         *
         * Separate from setMaxAnisotropy because the two are independent: a
         * caller re-applying a filtering mode every frame must not disturb the
         * degree, and vice versa. Both early-out when nothing changed.
         *
         * @param minFilter Minifying filter.
         * @param magFilter Magnifying filter.
         */
        void setFilter(TextureMinFilter minFilter, TextureMagFilter magFilter);

        /**
         * @brief Set how far the sampler may stretch its footprint when minifying.
         *
         * A surface seen edge-on covers a texel footprint far longer in one
         * direction than the other, and a single mip level cannot represent
         * both - so the hardware picks one for the wide axis and the detail
         * along the narrow one is lost. A degree above 1 lets it take that many
         * samples along the long axis instead.
         *
         * Clamped to maxSupportedAnisotropy(), so a level the driver does not
         * offer degrades to the highest it does rather than failing. A texture
         * whose minification filter walks no mip chain is left at 1 whatever is
         * asked: it has no mip selection to correct, and multi-tap filtering
         * would only soften content that was asked to stay sharp.
         *
         * @param maxAnisotropy Requested degree; values below 1 are treated as 1.
         */
        void setMaxAnisotropy(float maxAnisotropy);

        /**
         * @brief Load the texture from a file.
         * @param filePath        Path to file.
         * @param flipVertically  Optionally flip vertically.
         * @param srgb            Whether to use sRGB color space (default: false).
         * @return True on success, false otherwise.
         */
        bool loadFromFile(const std::string& filePath, bool flipVertically = true, bool srgb = false);

        /**
         * @brief Get the width of the texture.
         * @return Texture width in pixels.
         */
        uint32_t getWidth() const { return m_params.width; }

        /**
         * @brief Get the height of the texture.
         * @return Texture height in pixels.
         */
        uint32_t getHeight() const { return m_params.height; }

    private:
        /**
         * @brief Delete the texture and zero m_id.
         *
         * Idempotent, so it is safe on a moved-from texture or after a previous release().
         */
        void release() noexcept;

        void applyParameters() const;

    private:
        std::string m_name;
        std::string m_path;

        Texture2DParams m_params;
};

} // namespace Vkm::GL
