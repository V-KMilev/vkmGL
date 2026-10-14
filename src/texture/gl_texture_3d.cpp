#include "gl_texture_3d.h"

#include "gl_error_handle.h"

namespace Vkm::GL {

Texture3D::Texture3D(const std::string& name, const Texture3DParams& params)
    : GLObject(GL_TEXTURE_3D, GL_TEXTURE, 0)
    , m_name(name)
    , m_params(params) {
    VKM_GL_CHECK(glCreateTextures(GL_TEXTURE_3D, 1, &m_id));
    VKM_GL_CHECK(glTextureStorage3D(
        m_id,
        1,
        m_params.internalFormat,
        static_cast<GLsizei>(m_params.width),
        static_cast<GLsizei>(m_params.height),
        static_cast<GLsizei>(m_params.depth)
    ));

    const auto wrap = static_cast<GLint>(toGLenum(m_params.wrap));
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrap));
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrap));
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, wrap));
    const auto minFilter = static_cast<GLint>(toGLenum(m_params.minFilter));
    const auto magFilter = static_cast<GLint>(toGLenum(m_params.magFilter));
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, minFilter));
    VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, magFilter));
}

Texture3D::~Texture3D() {
    release();
}

Texture3D& Texture3D::operator=(Texture3D && other) noexcept {
    if (this != &other) {
        release();
        // Through the base, like Texture2D's: GLObject owns the id and the two
        // fields beside it (target, label namespace), and moving only the id by
        // hand left those two describing the object that was overwritten.
        GLObject::operator=(std::move(other));
        m_name   = std::move(other.m_name);
        m_params = other.m_params;
    }
    return *this;
}

void Texture3D::release() noexcept {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteTextures(1, &m_id));
    m_id = 0;
}

void Texture3D::bindSlot(uint32_t slot) const {
    VKM_GL_CHECK(glBindTextureUnit(slot, m_id));
}

void Texture3D::bindImage(uint32_t unit, GLenum access, int32_t level) const {
    // Layered = GL_TRUE exposes the whole volume as an image3D to the shader.
    VKM_GL_CHECK(glBindImageTexture(unit, m_id, level, GL_TRUE, 0, access, m_params.internalFormat));
}

void Texture3D::upload(const void* pixels) const {
    VKM_GL_CHECK(glTextureSubImage3D(
        m_id,
        0,
        0,
        0,
        0,
        static_cast<GLsizei>(m_params.width),
        static_cast<GLsizei>(m_params.height),
        static_cast<GLsizei>(m_params.depth),
        m_params.format,
        m_params.type,
        pixels
    ));
}

void Texture3D::download(void* pixels, uint32_t bytes) const {
    const auto size = static_cast<GLsizei>(bytes);
    VKM_GL_CHECK(glGetTextureImage(m_id, 0, m_params.format, m_params.type, size, pixels));
}

} // namespace Vkm::GL
