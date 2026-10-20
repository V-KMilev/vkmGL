#include "gl_texture_2d_array.h"

#include <algorithm>
#include <utility>

#include "gl_error_handle.h"
#include "l_assert.h"

namespace Vkm::GL {

namespace {

// Wrap, filter and anisotropy of @p params on texture @p id.
void applySampling(uint32_t id, const Texture2DArrayParams& params) {
    const auto set = [id](GLenum name, GLenum value) {
        VKM_GL_CHECK(glTextureParameteri(id, name, static_cast<GLint>(value)));
    };
    set(GL_TEXTURE_WRAP_S, toGLenum(params.wrapS));
    set(GL_TEXTURE_WRAP_T, toGLenum(params.wrapT));
    set(GL_TEXTURE_MIN_FILTER, toGLenum(params.minFilter));
    set(GL_TEXTURE_MAG_FILTER, toGLenum(params.magFilter));
    // The enum exists only with the extension on a 4.5 context; asking without it is an error.
    const float degree = std::min(std::max(params.maxAnisotropy, 1.0f), maxSupportedAnisotropy());
    if (degree > 1.0f) VKM_GL_CHECK(glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, degree));
}

} // namespace

Texture2DArray::Texture2DArray(const std::string& name, const Texture2DArrayParams& params)
    : GLObject(GL_TEXTURE_2D_ARRAY, GL_TEXTURE, 0)
    , m_name(name)
    , m_params(params)
{
    VKM_ASSERT(params.width > 0 && params.height > 0 && params.layers > 0 && params.levels > 0);
    VKM_GL_CHECK(glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_id));
    setLabel(m_name.c_str());
    VKM_GL_CHECK(glTextureStorage3D(
        m_id,
        static_cast<GLsizei>(m_params.levels),
        m_params.internalFormat,
        static_cast<GLsizei>(m_params.width),
        static_cast<GLsizei>(m_params.height),
        static_cast<GLsizei>(m_params.layers)
    ));
    if (m_params.viewFormat == GL_NONE) {
        applySampling(m_id, m_params);
        return;
    }
    // Generated, not created: glTextureView gives the object to a name never bound.
    VKM_GL_CHECK(glGenTextures(1, &m_view));
    VKM_GL_CHECK(glTextureView(
        m_view,
        GL_TEXTURE_2D_ARRAY,
        m_id,
        m_params.viewFormat,
        0,
        m_params.levels,
        0,
        m_params.layers
    ));
    VKM_GL_CHECK(glObjectLabel(GL_TEXTURE, m_view, -1, m_name.c_str()));
    applySampling(m_view, m_params);
}

Texture2DArray::~Texture2DArray() {
    release();
}

Texture2DArray::Texture2DArray(Texture2DArray && other) noexcept
    : GLObject(std::move(other))
    , m_name(std::move(other.m_name))
    , m_params(other.m_params)
    , m_view(std::exchange(other.m_view, 0u)) {}

Texture2DArray& Texture2DArray::operator=(Texture2DArray && other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
        m_name   = std::move(other.m_name);
        m_params = other.m_params;
        m_view   = std::exchange(other.m_view, 0u);
    }
    return *this;
}

void Texture2DArray::release() noexcept {
    if (m_view != 0) {
        VKM_GL_CHECK(glDeleteTextures(1, &m_view));
        m_view = 0;
    }
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteTextures(1, &m_id));
    m_id = 0;
}

void Texture2DArray::bindSlot(uint32_t slot) const {
    VKM_GL_CHECK(glBindTextureUnit(slot, m_view != 0 ? m_view : m_id));
}

void Texture2DArray::bindImage(uint32_t unit, GLenum access, uint32_t level) const {
    const auto at = static_cast<GLint>(level);
    VKM_GL_CHECK(glBindImageTexture(unit, m_id, at, GL_TRUE, 0, access, m_params.internalFormat));
}

void Texture2DArray::clear(uint32_t level, GLenum format, GLenum type, const void* value) const {
    VKM_GL_CHECK(glClearTexImage(m_id, static_cast<GLint>(level), format, type, value));
}

void Texture2DArray::generateMipmaps() const {
    VKM_GL_CHECK(glGenerateTextureMipmap(m_id));
}

} // namespace Vkm::GL
