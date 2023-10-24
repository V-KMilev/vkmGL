#include "gl_quad.h"

namespace Core {
	Quad::Quad() : BasicObject() {
		init();
	}

	Quad::Quad(const Quad& other) : BasicObject() {}

	Quad& Quad::operator = (const Quad& other) {
		if (this == &other) {
			return *this;
		}

		BasicObject::operator=(other);

		return *this;
	}

	bool Quad::init() {
		_mVB = std::make_shared<VertexBuffer>(&_mVertices[0], QuadVerticesSize * sizeof(float));
		_mIB = std::make_shared<IndexBuffer>(&_mIndices[0], QuadIndicesSize);
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

	unsigned int Quad::getID() const {
		return _mID.getID();
	}

	bool Quad::updateTexture(const Texture& texture) {
		return _mTexture->update(texture);
	}

	bool Quad::updateTexture(unsigned int textureID) {
		return _mTexture->update(textureID);
	}

	bool Quad::updateTexture(const std::string& file) {
		return _mTexture->update(file);
	}

	void Quad::draw(const Renderer &renderer, const Shader &shader, unsigned int drawType) const {
		renderer.draw(*_mVA, *_mIB, shader, drawType);
	}
};
