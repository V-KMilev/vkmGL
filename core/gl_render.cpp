#include "gl_render.h"

Renderer::Renderer() : mClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {}

void Renderer::clear() const {
	MY_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::clearColor() const {
	MY_GL_CHECK(
		glClearColor(
			mClearColor[0],
			mClearColor[1],
			mClearColor[2],
			mClearColor[3]
		)
	);
}

void Renderer::draw(
	const VertexArray &vertex_array,
	const IndexBuffer &index_buffer,
	const Shader &shader,
	unsigned int drawType
) const {
	shader.bind();

	vertex_array.bind();
	index_buffer.bind();

	MY_GL_CHECK(glDrawElements(drawType, index_buffer.getCount(), GL_UNSIGNED_INT, nullptr));

	// Can add unbindings but not necessary. Only for debuging
}

void Renderer::draw(
	const VertexArray &vertex_array,
	const Shader &shader,
	unsigned int drawType,
	unsigned int first,
	unsigned int count
) const {
	shader.bind();

	vertex_array.bind();

	MY_GL_CHECK(glDrawArrays(drawType, first, count));
}

void Renderer::draw(
	uint32_t count,
	const uint32_t *buffers
) const {
	MY_GL_CHECK(glDrawBuffers(count, buffers));
}


// void Renderer::drawQuad(
// 	const Shader &shader,
// 	unsigned int drawType,
// 	const Quad& quad
// ) const {
// 	draw(*mQuadVA, *mQuadIB, shader, drawType);
// }
