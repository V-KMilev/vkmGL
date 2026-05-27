#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "TEXTURE"

#include "gl_texture.h"

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNM

namespace Core {

    namespace {
        GLint inferInternalFormat(int channels, bool srgb) {
            if (srgb) {
                switch (channels) {
                    case 3: return GL_SRGB8;
                    case 4: return GL_SRGB8_ALPHA8;
                    default: return GL_SRGB8_ALPHA8;
                }
            }
            switch (channels) {
                case 1: return GL_R8;
                case 2: return GL_RG8;
                case 3: return GL_RGB8;
                case 4: return GL_RGBA8;
                default: return GL_RGBA8;
            }
        }

        GLenum inferFormat(int channels) {
            switch (channels) {
                case 1: return GL_RED;
                case 2: return GL_RG;
                case 3: return GL_RGB;
                case 4: return GL_RGBA;
                default: return GL_RGBA;
            }
        }
    } // namespace

    Texture2D::Texture2D(const std::string& name, const Texture2DParams& params)
        : GLObject(GL_TEXTURE_2D, GL_TEXTURE, 0)
        , m_name(name)
        , m_path()
        , m_params(params)
    {
        VKM_GL_CHECK(glGenTextures(1, &m_id));
        VKM_ASSERT(m_id != 0);

        bind();
        VKM_GL_CHECK(glTexImage2D(
            GL_TEXTURE_2D,
            0,
            m_params.internalFormat,
            static_cast<GLsizei>(m_params.width),
            static_cast<GLsizei>(m_params.height),
            0,
            m_params.format,
            m_params.type,
            m_params.data
        ));

        applyParameters();

        if (m_params.generateMipmaps) {
            VKM_GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
        }

        unbind();
    }

    Texture2D::Texture2D(const std::string& filePath, bool flipVertically, bool srgb)
        : GLObject(GL_TEXTURE_2D, GL_TEXTURE, 0)
        , m_path(filePath)
    {
        // Extract name from file path (filename without extension)
        size_t lastSlash = filePath.find_last_of("/\\");
        size_t lastDot = filePath.find_last_of('.');
        if (lastDot != std::string::npos && lastDot > lastSlash) {
            m_name = filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
        } else {
            m_name = filePath.substr(lastSlash + 1);
        }

        if (!loadFromFile(filePath, flipVertically, srgb)) {
            LOG_ERROR("Failed to load texture from file '%s' in constructor", filePath.c_str());
            // Initialize with default params even on failure
            m_params = Texture2DParams{};
        }
    }

    Texture2D::~Texture2D() {
        release();
    }

    Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
        if (this != &other) {
            release();
            GLObject::operator=(std::move(other));
            m_name   = std::move(other.m_name);
            m_path   = std::move(other.m_path);
            m_params = other.m_params;
        }
        return *this;
    }

    void Texture2D::release() noexcept {
        if (m_id == 0) return;
        VKM_GL_CHECK(glDeleteTextures(1, &m_id));
        m_id = 0;
    }

    void Texture2D::bind(GLenum target) const {
        GLenum t = (target == GL_NONE) ? m_target : target;
        VKM_GL_CHECK(glBindTexture(t, m_id));
    }

    void Texture2D::unbind(GLenum target) const {
        GLenum t = (target == GL_NONE) ? m_target : target;
        VKM_GL_CHECK(glBindTexture(t, 0));
    }

    void Texture2D::bindSlot(uint32_t slot, GLenum target) const {
        VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
        bind(target);
    }

    void Texture2D::setData(const void* data, uint32_t width, uint32_t height, GLenum format, GLenum type) {
        m_params.width = width;
        m_params.height = height;
        m_params.format = format;
        m_params.type = type;

        // Infer internal format from data format if not already set properly
        GLint internalFormat = m_params.internalFormat;
        if (internalFormat == GL_RGBA8) {
            // Auto-detect based on format
            switch (format) {
                case GL_RED:
                    internalFormat = GL_R8;
                    break;
                case GL_RG:
                    internalFormat = GL_RG8;
                    break;
                case GL_RGB:
                    internalFormat = GL_RGB8;
                    break;
                case GL_RGBA:
                    internalFormat = GL_RGBA8;
                    break;
                case GL_SRGB:
                    internalFormat = GL_SRGB8;
                    break;
                case GL_SRGB_ALPHA:
                    internalFormat = GL_SRGB8_ALPHA8;
                    break;
                default:
                    internalFormat = GL_RGBA8;
                    break;
            }
        }

        bind();
        VKM_GL_CHECK(glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internalFormat,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            0,
            format,
            type,
            data
        ));

        m_params.internalFormat = internalFormat;

        if (m_params.generateMipmaps) {
            VKM_GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
        }
        unbind();
    }

    void Texture2D::setWrap(TextureWrap s, TextureWrap t) {
        m_params.wrapS = s;
        m_params.wrapT = t;
        bind();
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGL(m_params.wrapS)));
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGL(m_params.wrapT)));
        unbind();
    }

    void Texture2D::setFilter(TextureMinFilter minFilter, TextureMagFilter magFilter) {
        m_params.minFilter = minFilter;
        m_params.magFilter = magFilter;
        bind();
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGL(m_params.minFilter)));
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGL(m_params.magFilter)));
        unbind();
    }

    bool Texture2D::loadFromFile(const std::string& filePath, bool flipVertically, bool srgb) {
        m_path = filePath;

        // Initialize default parameters if not already set
        if (m_params.width == 0 && m_params.height == 0) {
            m_params = Texture2DParams{};
        }

        stbi_set_flip_vertically_on_load(flipVertically);

        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

        if (!data) {
            LOG_ERROR("Failed to load texture '%s': %s", filePath.c_str(), stbi_failure_reason());
            return false;
        }

        m_params.width = static_cast<uint32_t>(width);
        m_params.height = static_cast<uint32_t>(height);

        GLenum format = inferFormat(channels);
        GLint internalFormat = inferInternalFormat(channels, srgb);
        m_params.format = format;
        m_params.type = GL_UNSIGNED_BYTE;
        m_params.internalFormat = internalFormat;

        // Generate texture ID if not already created
        if (m_id == 0) {
            VKM_GL_CHECK(glGenTextures(1, &m_id));
            VKM_ASSERT(m_id != 0);
        }

        bind();
        VKM_GL_CHECK(glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internalFormat,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data
        ));

        applyParameters();

        if (m_params.generateMipmaps) {
            VKM_GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
        }

        unbind();
        stbi_image_free(data);
        return true;
    }

    void Texture2D::applyParameters() const {
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGL(m_params.wrapS)));
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGL(m_params.wrapT)));
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGL(m_params.minFilter)));
        VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGL(m_params.magFilter)));
    }

    GLenum Texture2D::toGL(TextureWrap wrap) {
        switch (wrap) {
            case TextureWrap::Repeat:         return GL_REPEAT;
            case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
            case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
            case TextureWrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
        }
        return GL_CLAMP_TO_EDGE;
    }

    GLenum Texture2D::toGL(TextureMinFilter filter) {
        switch (filter) {
            case TextureMinFilter::Nearest:               return GL_NEAREST;
            case TextureMinFilter::Linear:                return GL_LINEAR;
            case TextureMinFilter::NearestMipmapNearest:  return GL_NEAREST_MIPMAP_NEAREST;
            case TextureMinFilter::LinearMipmapNearest:   return GL_LINEAR_MIPMAP_NEAREST;
            case TextureMinFilter::NearestMipmapLinear:   return GL_NEAREST_MIPMAP_LINEAR;
            case TextureMinFilter::LinearMipmapLinear:    return GL_LINEAR_MIPMAP_LINEAR;
        }
        return GL_LINEAR_MIPMAP_LINEAR;
    }

    GLenum Texture2D::toGL(TextureMagFilter filter) {
        switch (filter) {
            case TextureMagFilter::Nearest: return GL_NEAREST;
            case TextureMagFilter::Linear:  return GL_LINEAR;
        }
        return GL_LINEAR;
    }
};
