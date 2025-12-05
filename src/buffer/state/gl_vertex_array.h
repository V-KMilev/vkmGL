#pragma once

#include "gl_object.h"
#include "gl_vertex_buffer_layout.h"
#include "gl_vertex_buffer.h"

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

        VertexArray(VertexArray && other) = delete;
        VertexArray& operator=(VertexArray && other) = delete;

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
         * 
         * This will enable and specify the vertex attributes described in the layout.
         *
         * @param vertexBuffer The vertex buffer to associate with this VAO.
         * @param layout The layout describing the structure of each vertex.
         */
        void addBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout);

        /**
         * @brief Adds a vertex buffer and its layout to this VAO, starting at a specified attribute index.
         * 
         * Allows multiple buffers/layouts to be assigned to different starting attribute indices.
         *
         * @param vertexBuffer The vertex buffer to add.
         * @param layout The layout of the vertex data.
         * @param startIndex The starting attribute index in the VAO.
         */
        void addBuffer(const VertexBuffer& vertexBuffer, const VertexBufferLayout& layout, uint32_t startIndex);

        /**
         * @brief Sets the attribute divisor for instanced rendering for a given attribute index.
         *
         * Controls how frequently a vertex attribute advances during instanced rendering. A divisor of 0 means
         * the attribute is per-vertex. A divisor of 1 means per-instance, and so on.
         *
         * @param index Attribute index to set the divisor for.
         * @param divisor The divisor value to set for instancing.
         */
        void setAttributeDivisor(uint32_t index, uint32_t divisor);

    private:
        uint32_t m_attributeIndex;
};
};
