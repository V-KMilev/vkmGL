#pragma once

#include <cstdint>
#include <string>
#include <GL/glew.h>

#include "gl_object.h"

namespace Core {

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
enum class TextureMinFilter {
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
enum class TextureMagFilter {
    Nearest,    ///< Nearest texel
    Linear      ///< Linear interpolation
};

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
        void bind(GLenum target = GL_NONE) const override;

        /**
         * @brief Unbind the texture from the bind point.
         * @param target Target texture type, defaults to GL_NONE (use internal target).
         */
        void unbind(GLenum target = GL_NONE) const override;

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
         * @param minFilter Minifying filter.
         * @param magFilter Magnifying filter.
         */
        void setFilter(TextureMinFilter minFilter, TextureMagFilter magFilter);

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
        /// Delete the texture and zero m_id. Idempotent - safe on a
        /// moved-from texture or after a previous release().
        void release() noexcept;

        void applyParameters() const;

        // Convert to underlying OpenGL enums
        static GLenum toGL(TextureWrap wrap);
        static GLenum toGL(TextureMinFilter filter);
        static GLenum toGL(TextureMagFilter filter);

    private:
        std::string m_name;
        std::string m_path;

        Texture2DParams m_params;
};

} // namespace Core
