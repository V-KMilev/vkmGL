#include "gl_texture.h"

#include <utility>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNM

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	Texture::~Texture() {
		if(_mID != 0) {
			MY_GL_CHECK(glDeleteTextures(1, &_mID));
		}
	}

	Texture::Texture(const std::string& file) :
		_mID(0),
		_mPath(file),
		_mName(""),
		_mSource(TextureSource::FILE),
		_mData(nullptr),
		_mWidth(0),
		_mHeight(0),
		_mComponentsPerPixel(0)
	{
		init();
	}

	Texture::Texture(const std::string& name, void* data) :
		_mID(0),
		_mPath(""),
		_mName(name),
		_mSource(TextureSource::RAW),
		_mData(static_cast<unsigned char*>(data)),
		_mWidth(0),
		_mHeight(0),
		_mComponentsPerPixel(0)
	{
		init();
	}

	Texture::Texture(const Texture& other) {
		_mPath               = other._mPath;
		_mName               = other._mName;
		_mSource             = other._mSource;
		_mData               = other._mData;
		_mWidth              = other._mWidth;
		_mHeight             = other._mHeight;
		_mComponentsPerPixel = other._mComponentsPerPixel;
		_mID                 = other._mID;
	}

	Texture& Texture::operator = (const Texture& other) {
		if(this == &other) {
			return *this;
		}

		_mPath               = other._mPath;
		_mName               = other._mName;
		_mSource             = other._mSource;
		_mData               = other._mData;
		_mWidth              = other._mWidth;
		_mHeight             = other._mHeight;
		_mComponentsPerPixel = other._mComponentsPerPixel;
		_mID                 = other._mID;

		return *this;
	}

	Texture::Texture(Texture && other) {
		_mPath               = std::move(other._mPath);
		_mName               = std::move(other._mName);
		_mSource             = std::move(other._mSource);
		_mData               = std::move(other._mData);
		_mWidth              = std::move(other._mWidth);
		_mHeight             = std::move(other._mHeight);
		_mComponentsPerPixel = std::move(other._mComponentsPerPixel);
		_mID                 = std::move(other._mID);
	}

	Texture& Texture::operator = (Texture && other) {
		if(this == &other) {
			return *this;
		}

		_mPath               = std::move(other._mPath);
		_mName               = std::move(other._mName);
		_mSource             = std::move(other._mSource);
		_mData               = std::move(other._mData);
		_mWidth              = std::move(other._mWidth);
		_mHeight             = std::move(other._mHeight);
		_mComponentsPerPixel = std::move(other._mComponentsPerPixel);
		_mID                 = std::move(other._mID);

		return *this;
	}

	void Texture::bind(uint32_t slot, uint32_t type) const {
		MY_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
		MY_GL_CHECK(glBindTexture(type, _mID));
	};

	void Texture::unbind() const {
		MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	};

	bool Texture::init() {
		if (_mSource == TextureSource::FILE) {
			// Check if the path is empty
			if (_mPath.empty()) {
				printf("[WARN:CORE] Empty texture path.\n");
				return false;
			}
#ifdef _WIN32
			_mName = _mPath.substr(1, _mPath.find_last_of('\\'));
#else
			_mName = _mPath.substr(1, _mPath.find_last_of('/'));
#endif

			// Flip the texture vertically (OpenGL starts textures from the bottom)
			stbi_set_flip_vertically_on_load(true);
		}
		else if(_mSource == TextureSource::RAW) {

			if(_mData) {
				// Flip the texture vertically (OpenGL starts textures from the bottom)
				stbi__vertical_flip(_mData, _mWidth, _mHeight, sizeof(float) * 3);
			}
		}

		// Generate a new texture
		MY_GL_CHECK(glGenTextures(1, &_mID));
		M_ASSERT(_mID != 0);

		bind();

		if (!_mData) {
			// Loading the texture from the specified path
			_mData = stbi_load(_mPath.c_str(), &_mWidth, &_mHeight, &_mComponentsPerPixel, _mBytesPerPixel);
		}

		// Check if the texture buffer is empty (texture load failed)
		if (!_mData) {
			printf("[WARN:CORE] Failed to load texture '%s'. Error: %s\n", _mPath.c_str(), stbi_failure_reason());

			_mComponentsPerPixel = 0;
			_mWidth              = 0;
			_mHeight             = 0;

			return false;
		}

		MY_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _mWidth, _mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _mData));

		// Free the image data that is no longer needed
		stbi_image_free(_mData);

		// Set texture parameters for minification and magnification filters, and wrapping behavior
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // GL_TEXTURE_WRAP_S: horizontal
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)); // GL_TEXTURE_WRAP_T: vertical
		return true;
	}

	const std::string& Texture::getPath() const {
		return _mPath;
	}

	const std::string& Texture::getName() const {
		return _mName;
	}

	void Texture::clear() {
		if(_mID != 0) {
			MY_GL_CHECK(glDeleteTextures(1, &_mID));
			_mID = 0;
		}
	}

	bool Texture::update() {
		return init();
	}

	bool Texture::update(const Texture& texture) {
		*this = texture;
		return init();
	}

	bool Texture::update(unsigned int textureID) {
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Get the width and height of the texture
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &_mWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &_mHeight);

		// Read the pixel data from the texture
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &_mData);

		_mSource = TextureSource::RAW;

		return init();
	}

	bool Texture::update(const std::string& file) {
		_mPath = file;
		_mSource = TextureSource::FILE;
		return init();
	}

	bool Texture::update(const std::string& name, void* data) {
		_mName = name;
		_mData = static_cast<unsigned char*>(data);
		_mSource = TextureSource::RAW;

		return init();
	}
};