#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "TEXTURE"

#include "gl_texture_3d.h"

#include "gl_error_handle.h"

namespace VkmGL {

Texture3D::Texture3D(const std::string& name, const Texture3DParams& params)
    : GLObject(GL_TEXTURE_3D, GL_TEXTURE, 0)
    , m_name(name)
    , m_params(params) {
    VKM_GL_CHECK(glGenTextures(1, &m_id));
    VKM_GL_CHECK(glBindTexture(GL_TEXTURE_3D, m_id));

    VKM_GL_CHECK(glTexImage3D(
        GL_TEXTURE_3D, 0,
        static_cast<GLint>(m_params.internalFormat),
        static_cast<GLsizei>(m_params.width),
        static_cast<GLsizei>(m_params.height),
        static_cast<GLsizei>(m_params.depth),
        0, m_params.format, m_params.type, nullptr));

    const GLenum wrap = toGLenum(m_params.wrap);
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrap)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrap)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, static_cast<GLint>(wrap)));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(toGLenum(m_params.minFilter))));
    VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(toGLenum(m_params.magFilter))));

    VKM_GL_CHECK(glBindTexture(GL_TEXTURE_3D, 0));
}

Texture3D::~Texture3D() {
    release();
}

Texture3D& Texture3D::operator=(Texture3D && other) noexcept {
    if (this != &other) {
        release();
        m_id     = other.m_id;
        m_name   = std::move(other.m_name);
        m_params = other.m_params;
        other.m_id = 0;
    }
    return *this;
}

void Texture3D::release() noexcept {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

void Texture3D::bind(GLenum target) const {
    const GLenum t = (target == GL_NONE) ? GL_TEXTURE_3D : target;
    VKM_GL_CHECK(glBindTexture(t, m_id));
}

void Texture3D::unbind(GLenum target) const {
    const GLenum t = (target == GL_NONE) ? GL_TEXTURE_3D : target;
    VKM_GL_CHECK(glBindTexture(t, 0));
}

void Texture3D::bindSlot(uint32_t slot) const {
    VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
    VKM_GL_CHECK(glBindTexture(GL_TEXTURE_3D, m_id));
}

void Texture3D::bindImage(uint32_t unit, GLenum access, int32_t level) const {
    // Layered = GL_TRUE exposes the whole volume as an image3D to the shader.
    VKM_GL_CHECK(glBindImageTexture(unit, m_id, level, GL_TRUE, 0, access, m_params.internalFormat));
}

} // namespace VkmGL
