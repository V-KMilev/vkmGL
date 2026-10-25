#pragma once

#include "gl_object.h"
#include "gl_buffer.h"  // BufferSpan
#include "gl_vertex_buffer_layout.h"
#include "gl_vertex_buffer.h"
#include <cstdint>

namespace Vkm::GL {

class GLBuffer;

/// How a vertex attribute's stored values reach the shader.
enum class AttributeKind {
    Float,       ///< Converted to float as they are: a float, or an integer's value.
    Normalized,  ///< An integer mapped to 0..1 (unsigned) or -1..1 (signed).
    Integer      ///< Kept an integer, for an int or uint input; past 2^24 a float would lose it.
};

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
         * Each element goes through setAttribute: an integer type as AttributeKind::Integer, or
         * Normalized when the layout normalizes it, and every other type as Float.
         *
         * @param vertexBuffer Buffer to source the attributes from.
         * @param layout       Element list describing one vertex.
         * @param startIndex   First attribute location to write.
         */
        void addBuffer(
            const VertexBuffer& vertexBuffer,
            const VertexBufferLayout& layout,
            uint32_t startIndex = 0
        );

        /**
         * @brief Step the buffer fed at @p startIndex once every @p divisor instances, not per vertex.
         *
         * @param startIndex The startIndex that buffer was added at.
         * @param divisor    Instances per step; 0 steps per vertex.
         */
        void setDivisor(uint32_t startIndex, uint32_t divisor);

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

        /**
         * @brief Read attribute @p index from binding @p binding, and enable it.
         *
         * @param index   The shader input's location.
         * @param binding The binding its vertices come from (setVertexBuffer).
         * @param count   Values per vertex, 1..4.
         * @param type    GL type of each stored value, e.g. GL_FLOAT or GL_UNSIGNED_SHORT.
         * @param offset  Bytes into each vertex.
         * @param kind    How the values reach the shader.
         */
        void setAttribute(
            uint32_t index,
            uint32_t binding,
            int32_t count,
            GLenum type,
            uint32_t offset,
            AttributeKind kind = AttributeKind::Float
        );

        /**
         * @brief Feed binding @p binding from @p buffer, a vertex every @p stride bytes from
         *        @p offset.
         *
         * @param binding The binding.
         * @param buffer  The buffer read.
         * @param offset  Where the first vertex begins.
         * @param stride  Bytes from one vertex to the next.
         */
        void setVertexBuffer(uint32_t binding, const GLBuffer& buffer, uint32_t offset, uint32_t stride);

        /**
         * @brief Feed binding @p binding from a span of a stream, a vertex every @p stride bytes.
         *
         * @param binding The binding.
         * @param span    Where the vertices lie.
         * @param stride  Bytes from one vertex to the next.
         */
        void setVertexBuffer(uint32_t binding, const BufferSpan& span, uint32_t stride);

        /**
         * @brief Read indices from @p buffer.
         *
         * @param buffer The index buffer: 32-bit unsigned indices for the draws below.
         */
        void setIndexBuffer(const GLBuffer& buffer);

        /**
         * @brief Draw @p count indices from @p firstIndex, each offset by @p baseVertex, with
         *        this array bound.
         *
         * @param mode       The primitive, e.g. GL_TRIANGLES.
         * @param count      Indices drawn.
         * @param firstIndex The first index's place in the index buffer.
         * @param baseVertex Added to every index.
         */
        void drawElementsBaseVertex(
            GLenum mode,
            uint32_t count,
            uint32_t firstIndex,
            int32_t baseVertex
        ) const;

        /**
         * @brief Draw @p count indexed draws whose commands lie tightly packed in @p commands,
         *        with this array bound.
         *
         * Each command is five 32-bit values: count, instance count, first index, base vertex,
         * base instance. The indirect binding is left on @p commands' buffer.
         *
         * @param mode     The primitive, e.g. GL_TRIANGLES.
         * @param commands Where the first command lies.
         * @param count    How many commands.
         */
        void multiDrawElementsIndirect(GLenum mode, const BufferSpan& commands, uint32_t count) const;

        /**
         * @brief Draw @p count vertices from @p first, @p instances times, with this array bound.
         *
         * @param mode      The primitive.
         * @param first     The first vertex.
         * @param count     Vertices an instance.
         * @param instances How many instances.
         */
        void drawArraysInstanced(GLenum mode, int32_t first, int32_t count, int32_t instances) const;

        /**
         * @brief Draw @p draws runs of vertices, run i @p counts[i] from @p firsts[i], with this array
         *        bound.
         *
         * @param mode   The primitive.
         * @param firsts Each run's first vertex.
         * @param counts Each run's vertices.
         * @param draws  How many runs.
         */
        void multiDrawArrays(GLenum mode, const int32_t* firsts, const int32_t* counts, int32_t draws) const;

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
