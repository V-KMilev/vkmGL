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

    void Renderer::setClearColor(const glm::vec4& color) {
        m_clearColor = color;
    }

    void Renderer::enableDepthTest(bool enable) const {
        if (enable) {
            VKM_GL_CHECK(glEnable(GL_DEPTH_TEST));
        } else {
            VKM_GL_CHECK(glDisable(GL_DEPTH_TEST));
        }
    }

    void Renderer::setDepthFunc(GLenum func) const {
        VKM_GL_CHECK(glDepthFunc(func));
    }

    void Renderer::enableDepthWrite(bool enable) const {
        VKM_GL_CHECK(glDepthMask(enable ? GL_TRUE : GL_FALSE));
    }

    void Renderer::enableFaceCulling(bool enable) const {
        if (enable) {
            VKM_GL_CHECK(glEnable(GL_CULL_FACE));
        } else {
            VKM_GL_CHECK(glDisable(GL_CULL_FACE));
        }
    }

    void Renderer::setCullFace(GLenum face) const {
        VKM_GL_CHECK(glCullFace(face));
    }

    void Renderer::setFrontFace(GLenum winding) const {
        VKM_GL_CHECK(glFrontFace(winding));
    }

    void Renderer::enableBlending(bool enable) const {
        if (enable) {
            VKM_GL_CHECK(glEnable(GL_BLEND));
        } else {
            VKM_GL_CHECK(glDisable(GL_BLEND));
        }
    }

    void Renderer::setBlendFunc(GLenum src, GLenum dst) const {
        VKM_GL_CHECK(glBlendFunc(src, dst));
    }

    void Renderer::setBlendEquation(GLenum mode) const {
        VKM_GL_CHECK(glBlendEquation(mode));
    }

    void Renderer::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) const {
        VKM_GL_CHECK(glViewport(x, y, width, height));
    }

    void Renderer::enableScissor(bool enable) const {
        if (enable) {
            VKM_GL_CHECK(glEnable(GL_SCISSOR_TEST));
        } else {
            VKM_GL_CHECK(glDisable(GL_SCISSOR_TEST));
        }
    }

    void Renderer::setScissor(int32_t x, int32_t y, int32_t width, int32_t height) const {
        VKM_GL_CHECK(glScissor(x, y, width, height));
    }

    void Renderer::setPolygonMode(GLenum face, GLenum mode) const {
        VKM_GL_CHECK(glPolygonMode(face, mode));
    }

    void Renderer::setDefaultState() const {
        enableDepthTest(true);
        setDepthFunc(GL_LEQUAL);
        enableDepthWrite(true);

        enableFaceCulling(true);
        setCullFace(GL_BACK);
        setFrontFace(GL_CCW);

        enableBlending(false);
        setPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
            indexBuffer.getType(),
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
