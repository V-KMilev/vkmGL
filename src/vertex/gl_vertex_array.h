#pragma once

#include "gl_object.h"
#include "gl_vertex_buffer_layout.h"
#include "gl_vertex_buffer.h"
#include <cstdint>

namespace Vkm::GL {

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
         */
        void bind() const;

        /**
         * @brief Unbinds this vertex array object (VAO).
         */
        void unbind() const;

        /**
         * @brief Adds a vertex buffer and its layout to this VAO.
         *
         * The layout's elements land on consecutive attribute locations starting
         * at @p startIndex, so a VAO fed from two buffers gives the second one
         * the index the first one ended at.
         *
         * That index is the caller's to state and is not remembered between
         * calls: a VAO re-fed with a new buffer - a ring that outgrew itself,
         * say - must re-wire the same locations, and a counter that carried over
         * would move them instead and leave the old, deleted buffer wired to the
         * ones the shader actually reads.
         *
         * @param vertexBuffer Buffer to source the attributes from.
         * @param layout       Element list describing one vertex.
         * @param startIndex   First attribute location to write.
         */
        void addBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout,
                       uint32_t startIndex = 0);

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
        /**
         * @brief Delete the VAO and zero m_id.
         *
         * Idempotent, so it is safe on a moved-from VAO or after a previous
         * release().
         */
        void release() noexcept;
};

} // namespace Vkm::GL
