#pragma once

#include <cstdint>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace Vkm::GL {

/**
 * @brief Depth state configuration for OpenGL context.
 */
struct DepthState {
    bool enabled      = false;      ///< Enable/disable depth testing.
    GLenum func       = GL_LESS;    ///< Depth comparison function (e.g., GL_LESS, GL_EQUAL).
    bool writeEnabled = true;       ///< Enable/disable writing to depth buffer.
};

/**
 * @brief Face culling state configuration for OpenGL context.
 */
struct FaceCullState {
    bool enabled     = false;      ///< Enable/disable face culling.
    GLenum cullFace  = GL_BACK;    ///< Which face to cull (e.g., GL_BACK, GL_FRONT).
    GLenum frontFace = GL_CCW;     ///< Vertex winding order considered front-facing.
};

/**
 * @brief Blending state configuration for OpenGL context.
 */
struct BlendState {
    bool enabled     = false;          ///< Enable/disable blending.
    GLenum srcFactor = GL_ONE;         ///< Source blend factor.
    GLenum dstFactor = GL_ZERO;        ///< Destination blend factor.
    GLenum equation  = GL_FUNC_ADD;    ///< Blend equation/mode.
};

/**
 * @brief Stencil test + write configuration. Defaults mirror the GL spec
 *        so a freshly-constructed Context tracks the same state the
 *        driver starts in.
 */
struct StencilState {
    bool   enabled   = false;               ///< Enable/disable stencil test.
    GLenum func      = GL_ALWAYS;           ///< Comparison function.
    GLint  ref       = 0;                   ///< Reference value.
    GLuint funcMask  = 0xFFFFFFFFu;         ///< Mask AND-ed with ref and stencil during test.
    GLenum sfail     = GL_KEEP;             ///< Op when stencil test fails.
    GLenum dpfail    = GL_KEEP;             ///< Op when stencil passes, depth fails.
    GLenum dppass    = GL_KEEP;             ///< Op when stencil and depth pass.
    GLuint writeMask = 0xFFFFFFFFu;         ///< Bits writable in the stencil buffer.
};

/**
 * @brief Per-channel color write mask.
 */
struct ColorMaskState {
    bool r = true;
    bool g = true;
    bool b = true;
    bool a = true;
};

/**
 * @brief Raster (pipeline) state configuration, encompassing depth, culling, blending, and some additional features.
 */
struct RasterState {
    DepthState     depth;            ///< Depth state.
    FaceCullState  cull;             ///< Face culling state.
    BlendState     blend;            ///< Blending state.
    StencilState   stencil;          ///< Stencil test + write state.
    ColorMaskState colorMask;        ///< Per-channel color write mask.
    bool colorMaskSplit = false;     ///< A draw buffer's mask was set apart from colorMask.
    GLenum polygonMode = GL_FILL;    ///< Polygon mode (e.g., GL_FILL, GL_LINE).
    bool scissorEnabled = false;     ///< Enable/disable scissor test.

    bool     alphaToCoverage = false;     ///< Alpha read as multisample coverage.
    bool     depthClamp      = false;     ///< Depth clamped to the range rather than clipped.
    uint32_t clipDistances   = 0;         ///< Bit i: user clip plane i on.
    bool     offsetFill      = false;     ///< Polygon offset on filled polygons.
    bool     offsetLine      = false;     ///< On lines.
    bool     offsetPoint     = false;     ///< On points.
    float    offsetFactor    = 0.0f;      ///< Polygon offset's slope factor.
    float    offsetUnits     = 0.0f;      ///< Its constant, in depth units.
};

/**
 * @brief Core OpenGL context state manager and state change limiter.
 *
 * The Context class manages OpenGL state for depth, blending, face culling, scissor, viewport, and color clearing.
 * It reduces redundant state changes and allows centralized state alterations for performance.
 */
class Context {
    public:
        Context() = default;
        ~Context() = default;

        Context(const Context& other) = delete;
        Context& operator=(const Context& other) = delete;

        Context(Context && other) = delete;
        Context& operator=(Context && other) = delete;

        /**
         * @brief Set the color used when clearing the framebuffer.
         *
         * Stored only; applied to GL by the next clear() with the color bit
         * set (or an explicit clearColor() call).
         *
         * @param color RGBA color vector.
         */
        void setClearColor(const glm::vec4& color);

        /**
         * @brief Retrieve the current clear color.
         * @return Reference to current RGBA clear color vector.
         */
        const glm::vec4& getClearColor() const { return m_clearColor; }

        /**
         * @brief Commit the current clear color to the OpenGL context.
         */
        void clearColor() const;

        /**
         * @brief Clear framebuffer buffers (color/depth/stencil).
         * @param color      Clear color buffer if true.
         * @param depth      Clear depth buffer if true.
         * @param stencil    Clear stencil buffer if true.
         * @param depthValue What the depth buffer is cleared to: 1 is GL's far plane, 0 a reversed one's.
         */
        void clear(bool color = true, bool depth = true, bool stencil = false, float depthValue = 1.0f) const;

        /**
         * @brief Set the viewport region for rendering.
         *
         * Deliberately uncached, unlike the toggles and modes below - hence
         * const. Two reasons: the rectangle is set by code outside this class
         * (a mip chain sizes the viewport to the level it is rendering into),
         * so a cache here would go stale and start filtering out calls that
         * were needed; and the frame is GPU-bound, where removing a handful of
         * glViewport calls buys nothing measurable.
         *
         * @param x      Lower-left corner X coordinate.
         * @param y      Lower-left corner Y coordinate.
         * @param width  Width of the viewport.
         * @param height Height of the viewport.
         */
        void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) const;

        /**
         * @brief Enable or disable scissor testing.
         * @param enable True to enable, false to disable.
         */
        void enableScissor(bool enable);

        /**
         * @brief Check if scissor testing is enabled.
         * @return True if enabled.
         */
        bool isScissorEnabled() const { return m_state.scissorEnabled; }

        /**
         * @brief Set the scissor rectangle.
         * @param x      Lower-left corner X coordinate.
         * @param y      Lower-left corner Y coordinate.
         * @param width  Width of the rectangle.
         * @param height Height of the rectangle.
         */
        void setScissor(int32_t x, int32_t y, int32_t width, int32_t height) const;

        /**
         * @brief Enable or disable depth test.
         * @param enable True to enable, false to disable.
         */
        void setDepthTest(bool enable);

        /**
         * @brief Check if depth testing is enabled.
         * @return True if enabled.
         */
        bool isDepthTestEnabled() const { return m_state.depth.enabled; }

        /**
         * @brief Set the depth comparison function.
         * @param func OpenGL comparison function (e.g., GL_LESS, GL_GREATER).
         */
        void setDepthFunc(GLenum func);

        /**
         * @brief Get the currently set depth comparison function.
         * @return Currently used OpenGL comparison function.
         */
        GLenum getDepthFunc() const { return m_state.depth.func; }

        /**
         * @brief Enable or disable writing to the depth buffer.
         * @param enable True to enable, false to disable.
         */
        void setDepthWrite(bool enable);

        /**
         * @brief Check if depth writes are enabled.
         * @return True if enabled.
         */
        bool isDepthWriteEnabled() const { return m_state.depth.writeEnabled; }

        /**
         * @brief Enable or disable face culling.
         * @param enable True to enable, false to disable.
         */
        void setFaceCulling(bool enable);

        /**
         * @brief Check if face culling is enabled.
         * @return True if enabled.
         */
        bool isFaceCullingEnabled() const { return m_state.cull.enabled; }

        /**
         * @brief Set which faces to cull (front/back).
         * @param face OpenGL face enumeration.
         */
        void setCullFace(GLenum face);

        /**
         * @brief Get currently set face to cull.
         * @return OpenGL face enumeration.
         */
        GLenum getCullFace() const { return m_state.cull.cullFace; }

        /**
         * @brief Set which winding (CW/CCW) is considered front-facing.
         * @param winding OpenGL front face winding.
         */
        void setFrontFace(GLenum winding);

        /**
         * @brief Get the currently set front face winding.
         * @return OpenGL front face winding.
         */
        GLenum getFrontFace() const { return m_state.cull.frontFace; }

        /**
         * @brief Enable or disable blending.
         * @param enable True to enable, false to disable.
         */
        void setBlending(bool enable);

        /**
         * @brief Check if blending is enabled.
         * @return True if enabled.
         */
        bool isBlendingEnabled() const { return m_state.blend.enabled; }

        /**
         * @brief Set blending factors for source and destination.
         * @param src Source factor.
         * @param dst Destination factor.
         */
        void setBlendFunc(GLenum src, GLenum dst);

        /**
         * @brief Get currently set blend source factor.
         * @return Source factor enum.
         */
        GLenum getBlendSrc() const { return m_state.blend.srcFactor; }

        /**
         * @brief Get currently set blend destination factor.
         * @return Destination factor enum.
         */
        GLenum getBlendDst() const { return m_state.blend.dstFactor; }

        /**
         * @brief Set the blending equation (add/subtract/etc).
         * @param mode OpenGL blend equation enum.
         */
        void setBlendEquation(GLenum mode);

        /**
         * @brief Get the current blend equation.
         * @return OpenGL blend equation enum.
         */
        GLenum getBlendEquation() const { return m_state.blend.equation; }

        /**
         * @brief Enable or disable the stencil test.
         * @param enable True to enable, false to disable.
         */
        void setStencilTest(bool enable);

        /**
         * @brief Check if the stencil test is enabled.
         * @return True if enabled.
         */
        bool isStencilTestEnabled() const { return m_state.stencil.enabled; }

        /**
         * @brief Set the stencil comparison function, reference, and read mask.
         * @param func Comparison function (e.g., GL_ALWAYS, GL_NOTEQUAL).
         * @param ref  Reference value AND-ed with @p mask for the comparison.
         * @param mask Bits AND-ed with @p ref and the stencil value during test.
         */
        void setStencilFunc(GLenum func, GLint ref, GLuint mask);

        /**
         * @brief Set the stencil ops applied on test outcomes.
         * @param sfail  Op when the stencil test fails.
         * @param dpfail Op when stencil passes and depth fails.
         * @param dppass Op when stencil and depth both pass.
         */
        void setStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);

        /**
         * @brief Set the bits writable in the stencil buffer.
         * @param mask Write mask (0 = read-only, 0xFF = full 8-bit writes).
         */
        void setStencilMask(GLuint mask);

        /**
         * @brief Enable/disable per-channel writes to the color buffer.
         * @param r Enable writes to the red channel.
         * @param g Enable writes to the green channel.
         * @param b Enable writes to the blue channel.
         * @param a Enable writes to the alpha channel.
         */
        void setColorMask(bool r, bool g, bool b, bool a);

        /**
         * @brief Enable/disable per-channel writes to one draw buffer alone.
         *
         * The next setColorMask() then applies whatever it is given, as the buffers no longer
         * agree with what it last set.
         *
         * @param drawBuffer The draw buffer's index.
         * @param r          Enable writes to the red channel.
         * @param g          Enable writes to the green channel.
         * @param b          Enable writes to the blue channel.
         * @param a          Enable writes to the alpha channel.
         */
        void setColorMask(uint32_t drawBuffer, bool r, bool g, bool b, bool a);

        /**
         * @brief Read a fragment's alpha as the share of its samples it covers.
         *
         * Under multisampling a cutout's edge then blends rather than steps.
         *
         * @param enable True to enable, false to disable.
         */
        void setAlphaToCoverage(bool enable);

        /**
         * @brief Offset the depth of polygons rasterized as @p mode.
         *
         * Each fragment's depth moves by factor * its slope + units * the depth's resolution. One
         * offset holds for every mode; @p enable turns it on or off for @p mode alone.
         *
         * @param mode   GL_FILL, GL_LINE or GL_POINT.
         * @param enable True to offset that mode's polygons.
         * @param factor The slope's multiplier.
         * @param units  The constant, in the depth buffer's smallest steps.
         */
        void setPolygonOffset(GLenum mode, bool enable, float factor = 0.0f, float units = 0.0f);

        /**
         * @brief Clamp depth to the depth range rather than clip what lies past the near and far
         *        planes.
         *
         * @param enable True to clamp, false to clip.
         */
        void setDepthClamp(bool enable);

        /**
         * @brief Enable or disable user clip plane @p plane (gl_ClipDistance[plane]).
         *
         * @param plane  Below 8, the least every GL 4.5 driver offers.
         * @param enable True to clip by it.
         */
        void setClipDistance(uint32_t plane, bool enable);

        /**
         * @brief Where clip space's origin and depth range lie (glClipControl).
         *
         * Not cached: set once, as the depth convention the whole application keeps.
         *
         * @param origin GL_LOWER_LEFT or GL_UPPER_LEFT.
         * @param depth  GL_NEGATIVE_ONE_TO_ONE or GL_ZERO_TO_ONE.
         */
        void setClipControl(GLenum origin, GLenum depth) const;

        /**
         * @brief The byte alignment every row of pixels is packed and unpacked at.
         *
         * Not cached: set once. An alignment of 1 keeps a row of any width and channel count tight.
         *
         * @param alignment 1, 2, 4 or 8.
         */
        void setPixelAlignment(int32_t alignment) const;

        /**
         * @brief Filter a cube map across its faces' seams.
         *
         * Not cached: set once.
         *
         * @param enable True to filter across them.
         */
        void setSeamlessCubes(bool enable) const;

        /**
         * @brief Set the polygon rendering mode (fill/line/point).
         * @param face Polygon face(s) to apply mode to.
         * @param mode OpenGL polygon mode enum.
         */
        void setPolygonMode(GLenum face, GLenum mode);

        /**
         * @brief Get the currently set polygon mode.
         * @return Polygon mode enum.
         */
        GLenum getPolygonMode() const { return m_state.polygonMode; }

        /**
         * @brief Reset every tracked piece of state to its default.
         *
         * Covers all of it - depth, culling, blending, stencil, color mask,
         * scissor, polygon mode and offset, alpha to coverage, depth clamp and
         * clip planes - so a caller can rely on the name and not have to know
         * which subset was actually reset. The viewport and scissor *rectangles* are
         * not tracked and are left alone; see setViewport().
         */
        void setDefaultState();

        /**
         * @brief Maximum MSAA sample count the driver supports (GL_MAX_SAMPLES).
         *
         * Queried once on first use and cached - per-frame callers can clamp
         * requested sample counts without a driver round-trip.
         */
        int32_t maxSamples() const;

        /**
         * @brief Largest 2D texture edge the driver accepts (GL_MAX_TEXTURE_SIZE).
         *
         * Queried once on first use and cached, like maxSamples(). What an
         * atlas or a pyramid sized from authored settings clamps against: a
         * request past it is not a texture that renders badly, it is one the
         * driver refuses to allocate at all.
         *
         * @return The limit in texels; at least 1024 on any GL 4.5 context.
         */
        int32_t maxTextureSize() const;

        /**
         * @brief One of the driver's integer limits or state values (glGetIntegerv).
         *
         * Not cached: for the limits a host checks once, as it starts.
         *
         * @param name What is asked, e.g. GL_MAX_TEXTURE_IMAGE_UNITS.
         * @return Its value; 0 when the query failed.
         */
        int32_t integer(GLenum name) const;

        /**
         * @brief GL_VERSION string (e.g. "4.6.0 ..."), empty if unavailable.
         */
        std::string versionString() const;

        /**
         * @brief GL_RENDERER string (the GPU / device), empty if unavailable.
         */
        std::string rendererString() const;

    private:
        glm::vec4 m_clearColor{0.0f, 0.0f, 0.0f, 0.0f};  ///< Matches the GL default.
        RasterState m_state;
        mutable int32_t m_maxSamples     = -1;  ///< Cached GL_MAX_SAMPLES (-1 = not yet queried).
        mutable int32_t m_maxTextureSize = -1;  ///< Cached GL_MAX_TEXTURE_SIZE (-1 = not yet queried).
};

/**
 * @brief Order what earlier commands wrote before what later ones read (glMemoryBarrier).
 *
 * @param barriers GL_*_BARRIER_BIT, OR-ed: how the later commands read it.
 */
void memoryBarrier(GLbitfield barriers);

/// Wait until every command issued so far has finished (glFinish).
void finish();

} // namespace Vkm::GL
