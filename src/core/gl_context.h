#pragma once

#include <cstdint>

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Core {

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
 * @brief Raster (pipeline) state configuration, encompassing depth, culling, blending, and some additional features.
 */
struct RasterState {
    DepthState depth;                ///< Depth state.
    FaceCullState cull;              ///< Face culling state.
    BlendState blend;                ///< Blending state.
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
         * @brief Reset context state to sensible defaults (depth test on, culling on, blending off, etc).
         */
        void setDefaultState();

    private:
        glm::vec4 m_clearColor;
        RasterState m_state;
};

} // namespace Core