#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "TEXTURE"

#include "gl_texture.h"

#include <algorithm>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace Vkm::GL {

Texture2DParams renderTargetParams(
    uint32_t width,
    uint32_t height,
    GLenum internalFormat,
    GLenum format,
    TextureMinFilter minFilter,
    TextureMagFilter magFilter
) {
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
 * @brief Apply @p params' anisotropy to texture @p id.
 *
 * Skipped rather than set to 1.0 when the driver offers none: the enum is only
 * core in GL 4.6, so on the 4.5 context this library targets the call raises
 * GL_INVALID_ENUM rather than quietly doing nothing.
 *
 * @param id     The texture.
 * @param params Texture parameters to read the requested degree from.
 */
void applyAnisotropy(uint32_t id, const Texture2DParams& params) {
    if (maxSupportedAnisotropy() <= 1.0f) return;
    VKM_GL_CHECK(glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, effectiveAnisotropy(params)));
}

/**
 * @brief The extent of @p level of a texture @p size wide at level 0.
 *
 * @param size  Level 0's extent.
 * @param level The level.
 * @return At least 1.
 */
uint32_t levelExtent(uint32_t size, uint32_t level) {
    return std::max(size >> level, 1u);
}

} // namespace

uint32_t levelCount(const Texture2DParams& params) {
    if (params.levels > 0) return params.levels;
    if (!params.generateMipmaps) return 1;
    uint32_t levels = 1;
    for (uint32_t edge = std::max(params.width, params.height); edge > 1; edge >>= 1) ++levels;
    return levels;
}

Texture2D::Texture2D(const std::string& name, const Texture2DParams& params)
    : GLObject(GL_TEXTURE_2D, GL_TEXTURE, 0)
    , m_name(name)
    , m_params(params)
{
    allocate();
}

Texture2D::~Texture2D() {
    release();
}

Texture2D& Texture2D::operator=(Texture2D && other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
        m_name    = std::move(other.m_name);
        m_params  = other.m_params;
        m_swizzle = other.m_swizzle;
    }
    return *this;
}

void Texture2D::release() noexcept {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteTextures(1, &m_id));
    m_id = 0;
}

void Texture2D::allocate() {
    VKM_GL_CHECK(glCreateTextures(GL_TEXTURE_2D, 1, &m_id));
    VKM_ASSERT(m_id != 0);
    applyParameters();
    // A texture of no size - a stub for pixels still loading - has no storage to give, and
    // samples as an incomplete texture: black.
    if (m_params.width == 0 || m_params.height == 0) return;
    VKM_GL_CHECK(glTextureStorage2D(
        m_id,
        static_cast<GLsizei>(levelCount(m_params)),
        m_params.internalFormat,
        static_cast<GLsizei>(m_params.width),
        static_cast<GLsizei>(m_params.height)
    ));
    if (m_params.data) {
        uploadLevel(0, m_params.data);
        if (m_params.generateMipmaps) generateMipmaps();
    }
}

void Texture2D::bindSlot(uint32_t slot) const {
    VKM_GL_CHECK(glBindTextureUnit(slot, m_id));
}

void Texture2D::setData(const void* data, uint32_t width, uint32_t height, GLenum format, GLenum type) {
    m_params.format = format;
    m_params.type   = type;
    if (width != m_params.width || height != m_params.height) {
        release();
        m_params.width  = width;
        m_params.height = height;
        m_params.data   = nullptr;
        allocate();
    }
    uploadLevel(0, data);
    if (m_params.generateMipmaps) generateMipmaps();
}

void Texture2D::uploadLevel(uint32_t level, const void* pixels) const {
    VKM_ASSERT(level < levelCount(m_params));
    VKM_GL_CHECK(glTextureSubImage2D(
        m_id,
        static_cast<GLint>(level),
        0,
        0,
        static_cast<GLsizei>(levelExtent(m_params.width, level)),
        static_cast<GLsizei>(levelExtent(m_params.height, level)),
        m_params.format,
        m_params.type,
        pixels
    ));
}

void Texture2D::uploadCompressedLevel(uint32_t level, const void* blocks, uint32_t bytes) const {
    VKM_ASSERT(level < levelCount(m_params));
    VKM_GL_CHECK(glCompressedTextureSubImage2D(
        m_id,
        static_cast<GLint>(level),
        0,
        0,
        static_cast<GLsizei>(levelExtent(m_params.width, level)),
        static_cast<GLsizei>(levelExtent(m_params.height, level)),
        m_params.internalFormat,
        static_cast<GLsizei>(bytes),
        blocks
    ));
}

void Texture2D::generateMipmaps() const {
    VKM_GL_CHECK(glGenerateTextureMipmap(m_id));
}

void Texture2D::setSwizzle(const GLint rgba[4]) {
    std::copy(rgba, rgba + 4, m_swizzle.begin());
    VKM_GL_CHECK(glTextureParameteriv(m_id, GL_TEXTURE_SWIZZLE_RGBA, m_swizzle.data()));
}

void Texture2D::setFilter(TextureMinFilter minFilter, TextureMagFilter magFilter) {
    if (minFilter == m_params.minFilter && magFilter == m_params.magFilter) return;

    m_params.minFilter = minFilter;
    m_params.magFilter = magFilter;
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(toGLenum(minFilter))));
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(toGLenum(magFilter))));
    // The minification filter is the other half of what degree the driver ends
    // up applying, so a new one has to be pushed through it. setMaxAnisotropy
    // compares against the stored request and would not re-send it.
    applyAnisotropy(m_id, m_params);
}

void Texture2D::setMaxAnisotropy(float maxAnisotropy) {
    // Store the clamped degree, not the raw request, so this compare answers
    // "is the texture already filtered the way it was asked to be?" - callers
    // re-apply the same level over every texture they own each frame, and it
    // has to cost nothing when nothing moved.
    const float level = std::min(std::max(maxAnisotropy, 1.0f), maxSupportedAnisotropy());
    if (level == m_params.maxAnisotropy) return;

    m_params.maxAnisotropy = level;
    applyAnisotropy(m_id, m_params);
}

void Texture2D::setWrap(TextureWrap s, TextureWrap t) {
    m_params.wrapS = s;
    m_params.wrapT = t;
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, static_cast<GLint>(toGLenum(s))));
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, static_cast<GLint>(toGLenum(t))));
}

void Texture2D::applyParameters() const {
    const auto set = [this](GLenum name, GLenum value) {
        VKM_GL_CHECK(glTextureParameteri(m_id, name, static_cast<GLint>(value)));
    };
    set(GL_TEXTURE_WRAP_S, toGLenum(m_params.wrapS));
    set(GL_TEXTURE_WRAP_T, toGLenum(m_params.wrapT));
    set(GL_TEXTURE_MIN_FILTER, toGLenum(m_params.minFilter));
    set(GL_TEXTURE_MAG_FILTER, toGLenum(m_params.magFilter));
    applyAnisotropy(m_id, m_params);
    VKM_GL_CHECK(glTextureParameteriv(m_id, GL_TEXTURE_SWIZZLE_RGBA, m_swizzle.data()));
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

GLenum toGLenum(TextureCompare compare) {
    switch (compare) {
        case TextureCompare::None:         return GL_LEQUAL;
        case TextureCompare::Never:        return GL_NEVER;
        case TextureCompare::Less:         return GL_LESS;
        case TextureCompare::Equal:        return GL_EQUAL;
        case TextureCompare::LessEqual:    return GL_LEQUAL;
        case TextureCompare::Greater:      return GL_GREATER;
        case TextureCompare::NotEqual:     return GL_NOTEQUAL;
        case TextureCompare::GreaterEqual: return GL_GEQUAL;
        case TextureCompare::Always:       return GL_ALWAYS;
    }
    return GL_LEQUAL;
}

float maxSupportedAnisotropy() {
    // A driver constant, so it is asked for once. Without the cache the round
    // trip would repeat for every texture uploaded, and a scene has thousands.
    static const float SUPPORTED = queryMaxAnisotropy();
    return SUPPORTED;
}

} // namespace Vkm::GL
