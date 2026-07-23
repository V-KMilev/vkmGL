#include "gl_context.h"          // renamed header
#include "gl_error_handle.h"

namespace Core {

void Context::setClearColor(const glm::vec4& color) {
    m_clearColor = color;
}

void Context::clearColor() const {
    VKM_GL_CHECK(glClearColor(
        m_clearColor[0],
        m_clearColor[1],
        m_clearColor[2],
        m_clearColor[3]
    ));
}

void Context::clear(bool color, bool depth, bool stencil) const {
    GLbitfield mask = 0;
    if (color)   mask |= GL_COLOR_BUFFER_BIT;
    if (depth)   mask |= GL_DEPTH_BUFFER_BIT;
    if (stencil) mask |= GL_STENCIL_BUFFER_BIT;

    VKM_GL_CHECK(glClear(mask));
}

void Context::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) const {
    VKM_GL_CHECK(glViewport(x, y, width, height));
}

void Context::enableScissor(bool enable) {
    if (m_state.scissorEnabled == enable) return;
    m_state.scissorEnabled = enable;

    if (enable) { VKM_GL_CHECK(glEnable(GL_SCISSOR_TEST)); }
    else        { VKM_GL_CHECK(glDisable(GL_SCISSOR_TEST)); }
}

void Context::setScissor(int32_t x, int32_t y, int32_t width, int32_t height) const {
    VKM_GL_CHECK(glScissor(x, y, width, height));
}


void Context::setDepthTest(bool enable) {
    if (m_state.depth.enabled == enable) return;
    m_state.depth.enabled = enable;

    if (enable) { VKM_GL_CHECK(glEnable(GL_DEPTH_TEST)); }
    else        { VKM_GL_CHECK(glDisable(GL_DEPTH_TEST)); }
}

void Context::setDepthFunc(GLenum func) {
    if (m_state.depth.func == func) return;
    m_state.depth.func = func;
    VKM_GL_CHECK(glDepthFunc(func));
}

void Context::setDepthWrite(bool enable) {
    if (m_state.depth.writeEnabled == enable) return;
    m_state.depth.writeEnabled = enable;
    VKM_GL_CHECK(glDepthMask(enable ? GL_TRUE : GL_FALSE));
}


void Context::setFaceCulling(bool enable) {
    if (m_state.cull.enabled == enable) return;
    m_state.cull.enabled = enable;

    if (enable) { VKM_GL_CHECK(glEnable(GL_CULL_FACE)); }
    else        { VKM_GL_CHECK(glDisable(GL_CULL_FACE)); }
}

void Context::setCullFace(GLenum face) {
    if (m_state.cull.cullFace == face) return;
    m_state.cull.cullFace = face;
    VKM_GL_CHECK(glCullFace(face));
}

void Context::setFrontFace(GLenum winding) {
    if (m_state.cull.frontFace == winding) return;
    m_state.cull.frontFace = winding;
    VKM_GL_CHECK(glFrontFace(winding));
}


void Context::setBlending(bool enable) {
    if (m_state.blend.enabled == enable) return;
    m_state.blend.enabled = enable;

    if (enable) { VKM_GL_CHECK(glEnable(GL_BLEND)); }
    else        { VKM_GL_CHECK(glDisable(GL_BLEND)); }
}

void Context::setBlendFunc(GLenum src, GLenum dst) {
    if (m_state.blend.srcFactor == src && m_state.blend.dstFactor == dst) return;
    m_state.blend.srcFactor = src;
    m_state.blend.dstFactor = dst;
    VKM_GL_CHECK(glBlendFunc(src, dst));
}

void Context::setBlendEquation(GLenum mode) {
    if (m_state.blend.equation == mode) return;
    m_state.blend.equation = mode;
    VKM_GL_CHECK(glBlendEquation(mode));
}


void Context::setStencilTest(bool enable) {
    if (m_state.stencil.enabled == enable) return;
    m_state.stencil.enabled = enable;

    if (enable) { VKM_GL_CHECK(glEnable(GL_STENCIL_TEST)); }
    else        { VKM_GL_CHECK(glDisable(GL_STENCIL_TEST)); }
}

void Context::setStencilFunc(GLenum func, GLint ref, GLuint mask) {
    auto& s = m_state.stencil;
    if (s.func == func && s.ref == ref && s.funcMask == mask) return;
    s.func     = func;
    s.ref      = ref;
    s.funcMask = mask;
    VKM_GL_CHECK(glStencilFunc(func, ref, mask));
}

void Context::setStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) {
    auto& s = m_state.stencil;
    if (s.sfail == sfail && s.dpfail == dpfail && s.dppass == dppass) return;
    s.sfail  = sfail;
    s.dpfail = dpfail;
    s.dppass = dppass;
    VKM_GL_CHECK(glStencilOp(sfail, dpfail, dppass));
}

void Context::setStencilMask(GLuint mask) {
    if (m_state.stencil.writeMask == mask) return;
    m_state.stencil.writeMask = mask;
    VKM_GL_CHECK(glStencilMask(mask));
}

void Context::setColorMask(bool r, bool g, bool b, bool a) {
    auto& c = m_state.colorMask;
    if (c.r == r && c.g == g && c.b == b && c.a == a) return;
    c.r = r; c.g = g; c.b = b; c.a = a;
    VKM_GL_CHECK(glColorMask(
        r ? GL_TRUE : GL_FALSE,
        g ? GL_TRUE : GL_FALSE,
        b ? GL_TRUE : GL_FALSE,
        a ? GL_TRUE : GL_FALSE));
}

void Context::setPolygonMode(GLenum face, GLenum mode) {
    // Track only the common case (FRONT_AND_BACK) so callers can still set per-face if desired
    if (face == GL_FRONT_AND_BACK) m_state.polygonMode = mode;
    VKM_GL_CHECK(glPolygonMode(face, mode));
}

void Context::setDefaultState() {
    setDepthTest(true);
    setDepthFunc(GL_LEQUAL);
    setDepthWrite(true);

    setFaceCulling(true);
    setCullFace(GL_BACK);
    setFrontFace(GL_CCW);

    setBlending(false);
    setPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int32_t Context::maxSamples() const {
    if (m_maxSamples < 0) {
        GLint v = 1;
        glGetIntegerv(GL_MAX_SAMPLES, &v);
        m_maxSamples = v > 0 ? v : 1;
    }
    return m_maxSamples;
}

std::string Context::versionString() const {
    VKM_GL_CHECK(const GLubyte* v = glGetString(GL_VERSION));
    return v ? reinterpret_cast<const char*>(v) : std::string();
}

std::string Context::rendererString() const {
    VKM_GL_CHECK(const GLubyte* r = glGetString(GL_RENDERER));
    return r ? reinterpret_cast<const char*>(r) : std::string();
}

} // namespace Core
