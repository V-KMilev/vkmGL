#include "gl_object.h"

namespace Core {
	Object::Object() :
		_mTexture(nullptr),
		_mVA(nullptr),
		_mVB(nullptr),
		_mIB(nullptr),
		_mVBL(nullptr) {}

	Object::Object(const Object& other) {
		_mID = other._mID;

		_mWorldData = other._mWorldData;

		_mTexture = other._mTexture;

		_mVA  = other._mVA;
		_mVB  = other._mVB;
		_mIB  = other._mIB;
		_mVBL = other._mVBL;
	}

	Object& Object::operator=(const Object& other) {
		if (this == &other) {
			return *this; // Handle self-assignment
		}

		_mID = other._mID;

		_mWorldData = other._mWorldData;

		_mTexture = other._mTexture;

		_mVA  = other._mVA;
		_mVB  = other._mVB;
		_mIB  = other._mIB;
		_mVBL = other._mVBL;
		return *this;
	}

	unsigned int Object::getID() const {
		return _mID.getID();
	}

	const Texture& Object::getTexture() const {
		return *_mTexture;
	}

	bool Object::setTexture(const std::string& file) {
		_mTexture = nullptr;
		_mTexture = std::make_shared<Texture>(file);

		return _mTexture->update();
	}

	bool Object::setTexture(const Texture& texture) {
		_mTexture = nullptr;
		_mTexture = std::make_shared<Texture>(texture);

		return _mTexture->update();
	}

	void Object::draw(const Renderer& renderer, const Shader& shader, unsigned int drawType) const {
		shader.bind();

		_mTexture->bind(1);

		std::string texName = _mTexture->getName();
		shader.setUniform1i(texName, _mTexture->getID());

		renderer.draw(*_mVA, *_mIB, shader, drawType);
	}
};
