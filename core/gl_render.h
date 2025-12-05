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
