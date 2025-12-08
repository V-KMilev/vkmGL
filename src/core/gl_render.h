#pragma once

#include <cstdint>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "gl_index_buffer.h"
#include "gl_shader.h"
#include "gl_vertex_array.h"

namespace Core {

struct DepthState {
    bool enabled      = false;
    GLenum func       = GL_LESS;
    bool writeEnabled = true;
};

struct FaceCullState {
    bool enabled     = false;
    GLenum cullFace  = GL_BACK;
    GLenum frontFace = GL_CCW;
};

struct BlendState {
    bool enabled     = false;
    GLenum srcFactor = GL_ONE;
    GLenum dstFactor = GL_ZERO;
    GLenum equation  = GL_FUNC_ADD;
};

/**
 * @brief High-level OpenGL renderer abstraction.
 *
 * The Renderer class encapsulates basic drawing, clearing, and buffer management
 */
class Renderer {
    public:
        Renderer();
        ~Renderer() = default;

        Renderer(const Renderer& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;

        Renderer(Renderer && other) = delete;
        Renderer& operator=(Renderer && other) = delete;

    public:
        /**
         * @brief Clears the color and depth buffers.
         */
        void clear() const;

        /**
         * @brief Applies the current clear color (without clearing).
         * Call before clear() to set the clear color in OpenGL state.
         */
        void clearColor() const;

        /**
         * @brief Set the clear color used by clearColor()/clear().
         */
        void setClearColor(const glm::vec4& color);

        /**
         * @brief Enables or disables depth testing (GL_DEPTH_TEST).
         * @param enable Set to true to enable, false to disable.
         */
        void enableDepthTest(bool enable);

        /**
         * @brief Sets the depth comparison function (e.g., GL_LESS, GL_LEQUAL, etc).
         * @param func GLenum specifying depth function.
         */
        void setDepthFunc(GLenum func);

        /**
         * @brief Enables or disables writing to the depth buffer (via glDepthMask).
         * @param enable Set to true to enable depth writes, false to disable.
         */
        void enableDepthWrite(bool enable);

        /**
         * @brief Enables or disables face culling (GL_CULL_FACE).
         * @param enable Set to true to enable face culling, false to disable.
         */
        void enableFaceCulling(bool enable);

        /**
         * @brief Specifies which face to cull (e.g., GL_BACK, GL_FRONT, GL_FRONT_AND_BACK).
         * @param face GLenum specifying face.
         */
        void setCullFace(GLenum face);

        /**
         * @brief Sets the winding order to determine front-facing polygons (GL_CW or GL_CCW).
         * @param winding GLenum specifying winding order.
         */
        void setFrontFace(GLenum winding);

        /**
         * @brief Enables or disables blending (GL_BLEND).
         * @param enable Set to true to enable blending, false to disable.
         */
        void enableBlending(bool enable);

        /**
         * @brief Sets the blend function factors for source and destination.
         * @param src Source factor (e.g., GL_SRC_ALPHA).
         * @param dst Destination factor (e.g., GL_ONE_MINUS_SRC_ALPHA).
         */
        void setBlendFunc(GLenum src, GLenum dst);

        /**
         * @brief Sets the blend equation mode (e.g., GL_FUNC_ADD, GL_FUNC_SUBTRACT).
         * @param mode GLenum specifying blend equation.
         */
        void setBlendEquation(GLenum mode);

        /**
         * @brief Sets the viewport dimensions and position for rendering.
         * @param x      X coordinate of the lower left corner of the viewport.
         * @param y      Y coordinate of the lower left corner of the viewport.
         * @param width  Width of the viewport.
         * @param height Height of the viewport.
         */
        void setViewport(int32_t x, int32_t y, int32_t width, int32_t height) const;

        /**
         * @brief Enables or disables the scissor test (GL_SCISSOR_TEST).
         * @param enable Set to true to enable scissor test, false to disable.
         */
        void enableScissor(bool enable);

        /**
         * @brief Sets the scissor box dimensions for the scissor test.
         * @param x      X coordinate of the lower left corner of the scissor box.
         * @param y      Y coordinate of the lower left corner of the scissor box.
         * @param width  Width of the scissor box.
         * @param height Height of the scissor box.
         */
        void setScissor(int32_t x, int32_t y, int32_t width, int32_t height) const;

        /**
         * @brief Sets the polygon rasterization mode (e.g., fill, line, or point).
         * @param face  Specifies front and/or back facing polygons (GL_FRONT, GL_BACK, GL_FRONT_AND_BACK).
         * @param mode  Polygon mode (GL_FILL, GL_LINE, GL_POINT).
         */
        void setPolygonMode(GLenum face, GLenum mode);

        /**
         * @brief Apply a sane default pipeline state (depth test on/lequal, depth write on,
         *        backface cull CCW, polygon fill).
         */
        void setDefaultState();

        /**
         * @brief Draws indexed geometry using the provided vertex array, index buffer, and shader.
         * @param vertexArray The VertexArray object to bind and draw.
         * @param indexBuffer The IndexBuffer specifying indices to draw.
         * @param shader      The Shader to use for rendering.
         * @param drawType    Primitive type (e.g., GL_TRIANGLES).
         * @param indicesOffset Byte offset into the index buffer.
         */
        void draw(
            const VertexArray& vertexArray,
            const IndexBuffer& indexBuffer,
            const Shader& shader,
            uint32_t drawType = GL_TRIANGLES,
            uint32_t indicesOffset = 0
        ) const;

        /**
         * @brief Draws non-indexed geometry by calling glDrawArrays.
         * @param vertexArray The VertexArray object to bind and draw.
         * @param shader      The Shader to use for rendering.
         * @param drawType    Primitive type (e.g., GL_TRIANGLES).
         * @param first       Starting vertex index.
         * @param count       Number of vertices to draw.
         */
        void draw(
            const VertexArray& vertexArray,
            const Shader& shader,
            uint32_t drawType,
            uint32_t first,
            uint32_t count
        ) const;

        /**
         * @brief Set the color buffers to which fragment colors will be written (Multiple Render Targets).
         * @param count    Number of color buffers.
         * @param buffers  Array of buffer enums (e.g., GL_COLOR_ATTACHMENT0, ...).
         */
        void drawBuffers(
            uint32_t count,
            const uint32_t* buffers
        ) const;

        // State getters for querying current render state
        const glm::vec4& getClearColor() const { return m_clearColor; }
        // Individual getters for convenience
        bool isDepthTestEnabled() const { return depth.enabled; }
        GLenum getDepthFunc() const { return depth.func; }
        bool isDepthWriteEnabled() const { return depth.writeEnabled; }
        bool isFaceCullingEnabled() const { return faceCulling.enabled; }
        GLenum getCullFace() const { return faceCulling.cullFace; }
        GLenum getFrontFace() const { return faceCulling.frontFace; }
        bool isBlendingEnabled() const { return blending.enabled; }
        GLenum getBlendSrc() const { return blending.srcFactor; }
        GLenum getBlendDst() const { return blending.dstFactor; }
        GLenum getBlendEquation() const { return blending.equation; }
        GLenum getPolygonMode() const { return polygonMode; }
        bool isScissorEnabled() const { return scissorEnabled; }

    private:
        glm::vec4 m_clearColor;

        DepthState depth;
        FaceCullState faceCulling;
        BlendState blending;
        GLenum polygonMode = GL_FILL;
        bool scissorEnabled = false;
};
};
