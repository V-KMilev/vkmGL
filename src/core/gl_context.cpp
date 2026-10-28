#include "gl_context.h"

#include "gl_error_handle.h"

namespace Vkm::GL {

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

void Context::clear(bool color, bool depth, bool stencil, float depthValue) const {
    // A color clear applies the stored clear color first - setClearColor
    // alone never touches GL, so set + clear always clears with the caller's
    // color instead of whatever glClearColor state was left behind.
    if (color) clearColor();
    if (depth) VKM_GL_CHECK(glClearDepthf(depthValue));

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
    if (!m_state.colorMaskSplit && c.r == r && c.g == g && c.b == b && c.a == a) return;
    c.r = r; c.g = g; c.b = b; c.a = a;
    m_state.colorMaskSplit = false;
    VKM_GL_CHECK(glColorMask(
        r ? GL_TRUE : GL_FALSE,
        g ? GL_TRUE : GL_FALSE,
        b ? GL_TRUE : GL_FALSE,
        a ? GL_TRUE : GL_FALSE
    ));
}

void Context::setColorMask(uint32_t drawBuffer, bool r, bool g, bool b, bool a) {
    m_state.colorMaskSplit = true;
    VKM_GL_CHECK(glColorMaski(
        drawBuffer,
        r ? GL_TRUE : GL_FALSE,
        g ? GL_TRUE : GL_FALSE,
        b ? GL_TRUE : GL_FALSE,
        a ? GL_TRUE : GL_FALSE
    ));
}

namespace {

/// User clip planes setDefaultState() resets: the least every GL 4.5 driver offers.
constexpr uint32_t CLIP_PLANES = 8;

// Turn @p capability on or off when @p cached says otherwise, and remember it.
void toggle(bool& cached, GLenum capability, bool enable) {
    if (cached == enable) return;
    cached = enable;
    if (enable) { VKM_GL_CHECK(glEnable(capability)); }
    else        { VKM_GL_CHECK(glDisable(capability)); }
}

} // namespace

void Context::setAlphaToCoverage(bool enable) {
    toggle(m_state.alphaToCoverage, GL_SAMPLE_ALPHA_TO_COVERAGE, enable);
}

void Context::setPolygonOffset(GLenum mode, bool enable, float factor, float units) {
    if (enable && (m_state.offsetFactor != factor || m_state.offsetUnits != units)) {
        m_state.offsetFactor = factor;
        m_state.offsetUnits  = units;
        VKM_GL_CHECK(glPolygonOffset(factor, units));
    }
    switch (mode) {
        case GL_FILL:  toggle(m_state.offsetFill, GL_POLYGON_OFFSET_FILL, enable);   break;
        case GL_LINE:  toggle(m_state.offsetLine, GL_POLYGON_OFFSET_LINE, enable);   break;
        case GL_POINT: toggle(m_state.offsetPoint, GL_POLYGON_OFFSET_POINT, enable); break;
        default: break;
    }
}

void Context::setDepthClamp(bool enable) {
    toggle(m_state.depthClamp, GL_DEPTH_CLAMP, enable);
}

void Context::setClipDistance(uint32_t plane, bool enable) {
    const uint32_t bit = 1u << plane;
    bool on = (m_state.clipDistances & bit) != 0;
    toggle(on, GL_CLIP_DISTANCE0 + plane, enable);
    m_state.clipDistances = on ? (m_state.clipDistances | bit) : (m_state.clipDistances & ~bit);
}

void Context::setClipControl(GLenum origin, GLenum depth) const {
    VKM_GL_CHECK(glClipControl(origin, depth));
}

void Context::setPixelAlignment(int32_t alignment) const {
    VKM_GL_CHECK(glPixelStorei(GL_PACK_ALIGNMENT, alignment));
    VKM_GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
}

void Context::setSeamlessCubes(bool enable) const {
    if (enable) { VKM_GL_CHECK(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS)); }
    else        { VKM_GL_CHECK(glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS)); }
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
    setBlendFunc(GL_ONE, GL_ZERO);
    setBlendEquation(GL_FUNC_ADD);

    setStencilTest(false);
    setStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFFu);
    setStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    setStencilMask(0xFFFFFFFFu);

    setColorMask(true, true, true, true);
    enableScissor(false);

    setPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    setAlphaToCoverage(false);
    setPolygonOffset(GL_FILL, false);
    setPolygonOffset(GL_LINE, false);
    setPolygonOffset(GL_POINT, false);
    setDepthClamp(false);
    for (uint32_t plane = 0; plane < CLIP_PLANES; ++plane) setClipDistance(plane, false);
}

int32_t Context::maxSamples() const {
    if (m_maxSamples < 0) {
        GLint samples = 1;
        VKM_GL_CHECK(glGetIntegerv(GL_MAX_SAMPLES, &samples));
        m_maxSamples = samples > 0 ? samples : 1;
    }
    return m_maxSamples;
}

int32_t Context::maxTextureSize() const {
    if (m_maxTextureSize < 0) {
        // GL 4.5 guarantees 16384; the floor here is only for a query that
        // failed, so a caller clamping against it still gets a usable size.
        GLint size = 1024;
        VKM_GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size));
        m_maxTextureSize = size > 0 ? size : 1024;
    }
    return m_maxTextureSize;
}

int32_t Context::integer(GLenum name) const {
    GLint value = 0;
    VKM_GL_CHECK(glGetIntegerv(name, &value));
    return value;
}

void memoryBarrier(GLbitfield barriers) {
    VKM_GL_CHECK(glMemoryBarrier(barriers));
}

void finish() {
    VKM_GL_CHECK(glFinish());
}

std::string Context::versionString() const {
    const GLubyte* version = nullptr;
    VKM_GL_CHECK(version = glGetString(GL_VERSION));
    return version ? reinterpret_cast<const char*>(version) : std::string();
}

std::string Context::rendererString() const {
    const GLubyte* renderer = nullptr;
    VKM_GL_CHECK(renderer = glGetString(GL_RENDERER));
    return renderer ? reinterpret_cast<const char*>(renderer) : std::string();
}

} // namespace Vkm::GL
