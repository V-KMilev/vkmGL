#pragma once

#include <cstdint>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace VkmGL {

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
    GLenum polygonMode = GL_FILL;    ///< Polygon mode (e.g., GL_FILL, GL_LINE).
    bool scissorEnabled = false;     ///< Enable/disable scissor test.
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
         * @param color   Clear color buffer if true.
         * @param depth   Clear depth buffer if true.
         * @param stencil Clear stencil buffer if true.
         */
        void clear(bool color = true, bool depth = true, bool stencil = false) const;

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
         * Covers all of it - depth, culling, blending, stencil, colour mask and
         * scissor - so a caller can rely on the name and not have to know which
         * subset was actually reset. The viewport and scissor *rectangles* are
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
        mutable int32_t m_maxSamples = -1;  ///< Cached GL_MAX_SAMPLES (-1 = not yet queried).
};

} // namespace VkmGL
