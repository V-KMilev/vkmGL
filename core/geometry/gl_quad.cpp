#include "gl_quad.h"

namespace Core {
	Quad::Quad(uint32_t textureID) {
		init(textureID);
	}

	Quad::Quad(const Quad& other);
	Quad& Quad::operator = (const Quad& other);

	Quad::Quad(Quad && other);
	Quad& Quad::operator = (Quad && other);

	bool Quad::init(textureID) override {
		_mVB = std::make_shared<VertexBuffer>(&_mVertices[0], QuadVerticesSize * sizeof(float));
		_mIB = std::make_shared<IndexBuffer>(&m_Indices[0], QuadIndicesSize);
		_mVA = std::make_shared<VertexArray>();

		// 3 floats for the position, 2 floats for texture coordinates
		_mVBL = std::make_shared<VertexBufferLayout>();
		_mVBL->push<float>(3);
		_mVBL->push<float>(2);

		_mVA->addBuffer(*_mVB, *_mVBL);
	}

	bool Quad::deinit() override;
};
