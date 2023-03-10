#include "gl_render.h"

Renderer::Renderer()
	: mClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f))
{
////////////////////////////////////////////////////////////////////////////////////
	/* Cube data setup */

	mCubeVB = std::make_shared<VertexBuffer>(&mCubeVertices[0], CubeVerticesSize * sizeof(float));
	mCubeIB = std::make_shared<IndexBuffer>(&mCubeIndices[0], CubeIndicesSize);
	mCubeVA = std::make_shared<VertexArray>();

	mCubeLayout = std::make_shared<VertexBufferLayout>();
	// float float float - position
	mCubeLayout->push<float>(3);

	mCubeVA->addBuffer(*mCubeVB, *mCubeLayout);

////////////////////////////////////////////////////////////////////////////////////
	/* Quad data setup */

	mQuadVB = std::make_shared<VertexBuffer>(&mQuadVertices[0], QuadVerticesSize * sizeof(float));
	mQuadIB = std::make_shared<IndexBuffer>(&mQuadIndices[0], QuadIndicesSize);
	mQuadVA = std::make_shared<VertexArray>();

	mQuadLayout = std::make_shared<VertexBufferLayout>();
	// float float float - position
	// float float       - texCoords
	mQuadLayout->push<float>(3);
	mQuadLayout->push<float>(2);

	mQuadVA->addBuffer(*mQuadVB, *mQuadLayout);
}

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


void Renderer::drawCube(
	const Shader &shader,
	unsigned int drawType
) const {
	draw(*mCubeVA, *mCubeIB, shader, drawType);
}

void Renderer::drawQuad(
	const Shader &shader,
	unsigned int drawType
) const {
	draw(*mQuadVA, *mQuadIB, shader, drawType);
}
