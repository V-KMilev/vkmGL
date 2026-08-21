#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "TEXTURE"

#include "gl_texture.h"

#include <algorithm>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

// Both defines must precede the include - stb_image is header-only, so anything
// set after it has already missed the expansion it was meant to configure.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Vkm::GL {

Texture2DParams renderTargetParams(
    uint32_t width, uint32_t height, GLenum internalFormat, GLenum format,
    TextureMinFilter minFilter, TextureMagFilter magFilter) {
    Texture2DParams p;
    p.width           = width;
    p.height          = height;
    p.internalFormat  = internalFormat;
    p.format          = format;
    p.type            = GL_FLOAT;
    p.minFilter       = minFilter;
    p.magFilter       = magFilter;
    p.wrapS           = TextureWrap::ClampToEdge;
    p.wrapT           = TextureWrap::ClampToEdge;
    p.generateMipmaps = false;
    return p;
}


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

/**
 * @brief Ask the driver for its anisotropy ceiling.
 *
 * Either extension defines the same enum with the same value, so which one is
 * present only decides whether the query is legal at all.
 *
 * @return The reported maximum, or 1.0 when neither extension is present.
 */
float queryMaxAnisotropy() {
    if (!GLEW_ARB_texture_filter_anisotropic && !GLEW_EXT_texture_filter_anisotropic) {
        LOG_INFO("Anisotropic filtering unavailable here - textures stay trilinear");
        return 1.0f;
    }

    GLfloat limit = 1.0f;
    VKM_GL_CHECK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &limit));
    LOG_INFO("Anisotropic filtering available up to %.0fx", limit);
    return (limit > 1.0f) ? limit : 1.0f;
}

/**
 * @brief Whether @p filter reads from a mip chain rather than level 0 alone.
 *
 * @param filter Minification filter to classify.
 * @return True for the four Mipmap variants.
 */
bool samplesMipChain(TextureMinFilter filter) {
    switch (filter) {
        case TextureMinFilter::Nearest:              return false;
        case TextureMinFilter::Linear:               return false;
        case TextureMinFilter::NearestMipmapNearest: return true;
        case TextureMinFilter::LinearMipmapNearest:  return true;
        case TextureMinFilter::NearestMipmapLinear:  return true;
        case TextureMinFilter::LinearMipmapLinear:   return true;
    }
    return false;
}

/**
 * @brief The anisotropy degree @p params can actually be given.
 *
 * Two things can lower the request. The driver's ceiling is the obvious one.
 * The other is the texture's own minification filter: anisotropy corrects mip
 * *selection*, so a texture sampled from level 0 alone has nothing for it to
 * correct, and taking several taps across one that was deliberately left
 * unfiltered would soften the hard edges it was configured to keep.
 *
 * @param params Texture parameters carrying the request and the filter.
 * @return A degree of at least 1.0.
 */
float effectiveAnisotropy(const Texture2DParams& params) {
    if (!samplesMipChain(params.minFilter)) return 1.0f;
    return std::min(std::max(params.maxAnisotropy, 1.0f), maxSupportedAnisotropy());
}

/**
 * @brief Apply @p params' anisotropy to the currently bound 2D texture.
 *
 * Skipped rather than set to 1.0 when the driver offers none: the enum is only
 * core in GL 4.6, so on the 4.3 context this library targets the call raises
 * GL_INVALID_ENUM rather than quietly doing nothing.
 *
 * @param params Texture parameters to read the requested degree from.
 */
void applyAnisotropy(const Texture2DParams& params) {
    if (maxSupportedAnisotropy() <= 1.0f) return;
    VKM_GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY,
                                 effectiveAnisotropy(params)));
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
    // Stem of the path: filename without directory or extension. npos + 1
    // is 0, so a bare filename correctly starts at the beginning - but the
    // extension test has to treat "no directory" as position 0 rather than
    // comparing against npos, which no dot position can exceed.
    const size_t lastSlash = filePath.find_last_of("/\\");
    const size_t nameStart = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
    const size_t lastDot   = filePath.find_last_of('.');

    if (lastDot != std::string::npos && lastDot > nameStart) {
        m_name = filePath.substr(nameStart, lastDot - nameStart);
    } else {
        m_name = filePath.substr(nameStart);
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

Texture2D& Texture2D::operator=(Texture2D && other) noexcept {
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
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLenum(m_params.wrapS)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLenum(m_params.wrapT)));
    unbind();
}

void Texture2D::setFilter(TextureMinFilter minFilter, TextureMagFilter magFilter) {
    if (minFilter == m_params.minFilter && magFilter == m_params.magFilter) return;

    m_params.minFilter = minFilter;
    m_params.magFilter = magFilter;
    bind();
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLenum(m_params.minFilter)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLenum(m_params.magFilter)));
    unbind();
}

void Texture2D::setMaxAnisotropy(float maxAnisotropy) {
    // Store the clamped degree, not the raw request, so this compare answers
    // "is the texture already filtered the way it was asked to be?" - callers
    // re-apply the same level over every texture they own each frame, and it
    // has to cost nothing when nothing moved.
    const float level = std::min(std::max(maxAnisotropy, 1.0f), maxSupportedAnisotropy());
    if (level == m_params.maxAnisotropy) return;

    m_params.maxAnisotropy = level;
    bind();
    applyAnisotropy(m_params);
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
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLenum(m_params.wrapS)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLenum(m_params.wrapT)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLenum(m_params.minFilter)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLenum(m_params.magFilter)));
    applyAnisotropy(m_params);
}

GLenum toGLenum(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:         return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
    }
    return GL_CLAMP_TO_EDGE;
}

GLenum toGLenum(TextureMinFilter filter) {
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

GLenum toGLenum(TextureMagFilter filter) {
    switch (filter) {
        case TextureMagFilter::Nearest: return GL_NEAREST;
        case TextureMagFilter::Linear:  return GL_LINEAR;
    }
    return GL_LINEAR;
}

float maxSupportedAnisotropy() {
    // A driver constant, so it is asked for once. Without the cache the round
    // trip would repeat for every texture uploaded, and a scene has thousands.
    static const float SUPPORTED = queryMaxAnisotropy();
    return SUPPORTED;
}

} // namespace Vkm::GL
