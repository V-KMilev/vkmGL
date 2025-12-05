#pragma once

#include <cstdint>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "gl_index_buffer.h"
#include "gl_shader.h"
#include "gl_vertex_array.h"

namespace Core {

/**
 * @brief High-level OpenGL renderer abstraction.
 *
 * The Renderer class encapsulates basic drawing, clearing, and buffer management
 * operations for OpenGL. It provides utility methods for rendering indexed and 
 * non-indexed geometry through user-supplied VertexArray, IndexBuffer, and Shader 
 * abstractions. Renderer is non-copyable and non-movable to ensure unique 
 * management of its state.
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
        void enableDepthTest(bool enable) const;

        /**
         * @brief Sets the depth comparison function (e.g., GL_LESS, GL_LEQUAL, etc).
         * @param func GLenum specifying depth function.
         */
        void setDepthFunc(GLenum func) const;

        /**
         * @brief Enables or disables writing to the depth buffer (via glDepthMask).
         * @param enable Set to true to enable depth writes, false to disable.
         */
        void enableDepthWrite(bool enable) const;

        /**
         * @brief Enables or disables face culling (GL_CULL_FACE).
         * @param enable Set to true to enable face culling, false to disable.
         */
        void enableFaceCulling(bool enable) const;

        /**
         * @brief Specifies which face to cull (e.g., GL_BACK, GL_FRONT, GL_FRONT_AND_BACK).
         * @param face GLenum specifying face.
         */
        void setCullFace(GLenum face) const;

        /**
         * @brief Sets the winding order to determine front-facing polygons (GL_CW or GL_CCW).
         * @param winding GLenum specifying winding order.
         */
        void setFrontFace(GLenum winding) const;

        /**
         * @brief Enables or disables blending (GL_BLEND).
         * @param enable Set to true to enable blending, false to disable.
         */
        void enableBlending(bool enable) const;

        /**
         * @brief Sets the blend function factors for source and destination.
         * @param src Source factor (e.g., GL_SRC_ALPHA).
         * @param dst Destination factor (e.g., GL_ONE_MINUS_SRC_ALPHA).
         */
        void setBlendFunc(GLenum src, GLenum dst) const;

        /**
         * @brief Sets the blend equation mode (e.g., GL_FUNC_ADD, GL_FUNC_SUBTRACT).
         * @param mode GLenum specifying blend equation.
         */
        void setBlendEquation(GLenum mode) const;

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
        void enableScissor(bool enable) const;

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
        void setPolygonMode(GLenum face, GLenum mode) const;

        /**
         * @brief Apply a sane default pipeline state (depth test on/lequal, depth write on,
         *        backface cull CCW, polygon fill).
         */
        void setDefaultState() const;

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

    private:
        glm::vec4 m_clearColor;
};
};
