#include "gl_sampler.h"

#include "gl_error_handle.h"
#include "l_assert.h"

namespace Core {

namespace {

GLint toGLwrap(TextureWrap w) {
    switch (w) {
        case TextureWrap::Repeat:         return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:  return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

GLint toGLmin(TextureMinFilter f) {
    switch (f) {
        case TextureMinFilter::Nearest:              return GL_NEAREST;
        case TextureMinFilter::Linear:               return GL_LINEAR;
        case TextureMinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureMinFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case TextureMinFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
        case TextureMinFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR_MIPMAP_LINEAR;
}

GLint toGLmag(TextureMagFilter f) {
    return f == TextureMagFilter::Nearest ? GL_NEAREST : GL_LINEAR;
}

} // namespace

Sampler::Sampler() : Sampler(Params{}) {}

Sampler::Sampler(const Params& params)
    : GLObject(0, GL_SAMPLER, 0)
    , m_params(params)
{
    VKM_GL_CHECK(glGenSamplers(1, &m_id));
    VKM_ASSERT(m_id != 0);
    applyParams();
}

Sampler::~Sampler() {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteSamplers(1, &m_id));
}

Sampler& Sampler::operator=(Sampler&& other) noexcept {
    if (this != &other) {
        if (m_id != 0) {
            VKM_GL_CHECK(glDeleteSamplers(1, &m_id));
        }
        GLObject::operator=(std::move(other));
        m_params = other.m_params;
    }
    return *this;
}

void Sampler::bind(GLenum) const {}
void Sampler::unbind(GLenum) const {}

void Sampler::bindSlot(uint32_t slot) const {
    VKM_GL_CHECK(glBindSampler(slot, m_id));
}

void Sampler::unbindSlot(uint32_t slot) {
    VKM_GL_CHECK(glBindSampler(slot, 0));
}

void Sampler::setParams(const Params& params) {
    m_params = params;
    applyParams();
}

void Sampler::applyParams() {
    VKM_GL_CHECK(glSamplerParameteri(m_id, GL_TEXTURE_WRAP_S, toGLwrap(m_params.wrapS)));
    VKM_GL_CHECK(glSamplerParameteri(m_id, GL_TEXTURE_WRAP_T, toGLwrap(m_params.wrapT)));
    VKM_GL_CHECK(glSamplerParameteri(m_id, GL_TEXTURE_WRAP_R, toGLwrap(m_params.wrapR)));
    VKM_GL_CHECK(glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, toGLmin(m_params.minFilter)));
    VKM_GL_CHECK(glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, toGLmag(m_params.magFilter)));
}

} // namespace Core
