#pragma once

#include "gl_object.h"
#include "gl_vertex_buffer_layout.h"
#include "gl_vertex_buffer.h"
#include <cstdint>

namespace Core {

/**
 * @brief Vertex Array Object (VAO) wrapper.
 *
 * Manages vertex attribute state and binding of vertex/index buffers. Provides
 * helpers to add buffers with layouts and configure instancing divisors.
 */
class VertexArray : public GLObject {
    public:
        VertexArray();
        ~VertexArray() override;

        VertexArray(const VertexArray& other) = delete;
        VertexArray& operator=(const VertexArray& other) = delete;

        VertexArray(VertexArray && other) noexcept = default;
        VertexArray& operator=(VertexArray && other) noexcept;

    public:
        /**
         * @brief Binds this vertex array object (VAO) for subsequent OpenGL operations.
         * @param target Optional, unused for VAO but present for interface compatibility.
         */
        void bind(GLenum target = GL_NONE) const override;

        /**
         * @brief Unbinds this vertex array object (VAO).
         * @param target Optional, unused for VAO but present for interface compatibility.
         */
        void unbind(GLenum target = GL_NONE) const override;

        /**
         * @brief Adds a vertex buffer and its associated layout to this VAO, starting at attribute index 0.
         */
        void addBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout);

        /**
         * @brief Adds a vertex buffer and its layout to this VAO, starting at a specified attribute index.
         */
        void addBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout, uint32_t startIndex);

        /**
         * @brief Sets the attribute divisor for instanced rendering for a given attribute index.
         */
        void setAttributeDivisor(uint32_t index, uint32_t divisor);

        /**
         * @brief Draw @p count non-indexed vertices from this VAO with glDrawArrays.
         *
         * For geometry that has no index buffer (debug lines, point clouds). The
         * caller binds this VAO first.
         * @param mode  Primitive type (GL_LINES, GL_TRIANGLES, ...).
         * @param first First vertex to draw.
         * @param count Number of vertices.
         */
        void drawArrays(GLenum mode, int32_t first, int32_t count) const;

    private:
        /// Delete the VAO and zero m_id. Idempotent - safe on a moved-from
        /// VAO or after a previous release().
        void release() noexcept;

    private:
        uint32_t m_attributeIndex;
};

} // namespace Core
