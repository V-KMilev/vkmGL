#include "gl_render.h"

#include "gl_error_handle.h"

namespace Core {
    Renderer::Renderer()
        : m_clearColor(0.0f, 0.0f, 0.0f, 1.0f) {}

    void Renderer::clear() const {
        VKM_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void Renderer::clearColor() const {
        VKM_GL_CHECK(glClearColor(
            m_clearColor[0],
            m_clearColor[1],
            m_clearColor[2],
            m_clearColor[3]
        ));
    }

    void Renderer::draw(
        const VertexArray& vertexArray,
        const IndexBuffer& indexBuffer,
        const Shader& shader,
        uint32_t drawType,
        uint32_t indicesOffset
    ) const {
        shader.bind();
        vertexArray.bind();
        indexBuffer.bind();

        VKM_GL_CHECK(glDrawElements(
            drawType,
            static_cast<GLsizei>(indexBuffer.getCount()),
            GL_UNSIGNED_INT,
            reinterpret_cast<const void*>(static_cast<uintptr_t>(indicesOffset))
        ));
    }

    void Renderer::draw(
        const VertexArray& vertexArray,
        const Shader& shader,
        uint32_t drawType,
        uint32_t first,
        uint32_t count
    ) const {
        shader.bind();
        vertexArray.bind();

        VKM_GL_CHECK(glDrawArrays(
            drawType,
            static_cast<GLint>(first),
            static_cast<GLsizei>(count)
        ));
    }

    void Renderer::drawBuffers(
        uint32_t count,
        const uint32_t* buffers
    ) const {
        VKM_GL_CHECK(glDrawBuffers(
            static_cast<GLsizei>(count),
            reinterpret_cast<const GLenum*>(buffers)
        ));
    }
};
