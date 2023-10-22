#include "gl_triangle.h"

namespace Core {
	Triangle::Triangle(uint32_t textureID) {
		init(textureID);
	}

	Triangle::Triangle(const Triangle& other);
	Triangle& Triangle::operator = (const Triangle& other);

	Triangle::Triangle(Triangle && other);
	Triangle& Triangle::operator = (Triangle && other);

	bool Triangle::init(textureID) override {
		_mVB = std::make_shared<VertexBuffer>(&_mVertices[0], TriangleVerticesSize * sizeof(float));
		_mIB = std::make_shared<IndexBuffer>(&m_Indices[0], TriangleIndicesSize);
		_mVA = std::make_shared<VertexArray>();

		// 3 floats for position, 2 floats for texture coordinates
		_mVBL = std::make_shared<VertexBufferLayout>();
		_mVBL->push<float>(3);
		_mVBL->push<float>(2);

		_mVA->addBuffer(*_mVB, *_mVBL);
	}

	bool Triangle::deinit() override;
};
