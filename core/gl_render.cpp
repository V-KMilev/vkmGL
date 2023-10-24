#include "gl_render.h"

#include <utility>

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	Renderer::Renderer() : _mClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {}

	Renderer::Renderer(const Renderer& other) {
		this->_mClearColor = other._mClearColor;
	}

	Renderer& Renderer::operator = (const Renderer& other) {
		if (this == &other) {
			return *this;
		}

		this->_mClearColor = other._mClearColor;
		return *this;
	}

	Renderer::Renderer(Renderer && other) {
		this->_mClearColor = std::move(other._mClearColor);
	}

	Renderer& Renderer::operator = (Renderer && other) {
		if (this == &other) {
			return *this;
		}

		this->_mClearColor = std::move(other._mClearColor);
		return *this;
	}

	void Renderer::clear() const {
		MY_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void Renderer::clearColor() const {
		MY_GL_CHECK(
			glClearColor(
				_mClearColor[0],
				_mClearColor[1],
				_mClearColor[2],
				_mClearColor[3]
			)
		);
	}

	void Renderer::draw(
		const VertexArray& vertex_array,
		const IndexBuffer& index_buffer,
		const Shader &shader,
		unsigned int drawType
	) const {
		shader.bind();

		vertex_array.bind();
		index_buffer.bind();

		MY_GL_CHECK(glDrawElements(drawType, index_buffer.getCount(), GL_UNSIGNED_INT, nullptr));
	}

	void Renderer::draw(
		const VertexArray& vertex_array,
		const Shader& shader,
		unsigned int drawType,
		unsigned int first,
		unsigned int count
	) const {
		shader.bind();

		vertex_array.bind();

		MY_GL_CHECK(glDrawArrays(drawType, first, count));
	}

	void Renderer::draw(
		unsigned int count,
		const unsigned int* buffers
	) const {
		MY_GL_CHECK(glDrawBuffers(count, buffers));
	}
};