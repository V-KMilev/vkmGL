#include "gl_quad.h"

namespace Core {
	Quad::Quad() : Object() {
		init();
	}

	Quad::Quad(const Quad& other) : Object() {}

	Quad& Quad::operator = (const Quad& other) {
		if (this == &other) {
			return *this;
		}

		Object::operator=(other);

		return *this;
	}

	bool Quad::init() {
		_mVB = std::make_shared<VertexBuffer>(&_mVertices[0], (QuadVerticesSize + TextureVertices * QuadVertices) * sizeof(float));
		_mIB = std::make_shared<IndexBuffer>(&_mIndices[0], QuadIndices);
		_mVA = std::make_shared<VertexArray>();

		// 3 floats for position, 2 floats for texture coordinates
		_mVBL = std::make_shared<VertexBufferLayout>();
		_mVBL->push<float>(3);
		_mVBL->push<float>(2);

		_mVA->addBuffer(*_mVB, *_mVBL);

		return true;
	}

	bool Quad::deinit() {
		return true;
	}
};