#include "gl_triangle.h"

namespace Core {
	Triangle::Triangle() : BasicObject() {
		init();
	}

	Triangle::Triangle(const Triangle& other) : BasicObject() {}

	Triangle& Triangle::operator = (const Triangle& other) {
		if (this == &other) {
			return *this;
		}

		BasicObject::operator=(other);

		return *this;
	}

	bool Triangle::init() {
		_mVB = std::make_shared<VertexBuffer>(&_mVertices[0], TriangleVerticesSize * sizeof(float));
		_mIB = std::make_shared<IndexBuffer>(&_mIndices[0], TriangleIndicesSize);
		_mVA = std::make_shared<VertexArray>();

		// 3 floats for position, 2 floats for texture coordinates
		_mVBL = std::make_shared<VertexBufferLayout>();
		_mVBL->push<float>(3);
		_mVBL->push<float>(2);

		_mVA->addBuffer(*_mVB, *_mVBL);

		return true;
	}

	bool Triangle::deinit() {
		return true;
	}

	unsigned int Triangle::getID() const {
		return _mID.getID();
	}

	bool Triangle::updateTexture(const Texture& texture) {
		return _mTexture->update(texture);
	}

	bool Triangle::updateTexture(unsigned int textureID) {
		return _mTexture->update(textureID);
	}

	bool Triangle::updateTexture(const std::string& file) {
		return _mTexture->update(file);
	}

	void Triangle::draw(const Renderer &renderer, const Shader &shader, unsigned int drawType) const {
		renderer.draw(*_mVA, *_mIB, shader, drawType);
	}
};
