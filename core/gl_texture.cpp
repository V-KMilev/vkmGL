#include "gl_texture.h"

#include <utility>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNM

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	Texture::Texture() {
		// Generate a new texture
		MY_GL_CHECK(glGenTextures(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	Texture::~Texture() {
		if(_mID != 0) {
			MY_GL_CHECK(glDeleteTextures(1, &_mID));
		}
	}

	Texture::Texture(const std::string& file) :
		_mPath(file),
		_mSource(TextureSource::FILE)
	{
		// Generate a new texture
		MY_GL_CHECK(glGenTextures(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	Texture::Texture(const std::string& name, void* data) :
		_mName(name),
		_mSource(TextureSource::RAW),
		_mData(static_cast<unsigned char*>(data))
	{
		// Generate a new texture
		MY_GL_CHECK(glGenTextures(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	Texture::Texture(const Texture& other) {
		_mPath               = other._mPath;
		_mName               = other._mName;

		_mSource             = other._mSource;
		_mWrap               = other._mWrap;
		_mFilter             = other._mFilter;

		_mData               = other._mData;
		_mWidth              = other._mWidth;
		_mHeight             = other._mHeight;
		_mComponentsPerPixel = other._mComponentsPerPixel;
	}

	Texture& Texture::operator = (const Texture& other) {
		if(this == &other) {
			return *this;
		}

		_mPath               = other._mPath;
		_mName               = other._mName;

		_mSource             = other._mSource;
		_mWrap               = other._mWrap;
		_mFilter             = other._mFilter;

		_mData               = other._mData;
		_mWidth              = other._mWidth;
		_mHeight             = other._mHeight;
		_mComponentsPerPixel = other._mComponentsPerPixel;

		return *this;
	}

	Texture::Texture(Texture && other) {
		_mID                 = std::move(other._mID);
		_mPath               = std::move(other._mPath);
		_mName               = std::move(other._mName);

		_mSource             = std::move(other._mSource);
		_mWrap               = std::move(other._mWrap);
		_mFilter             = std::move(other._mFilter);

		_mData               = std::move(other._mData);
		_mWidth              = std::move(other._mWidth);
		_mHeight             = std::move(other._mHeight);
		_mComponentsPerPixel = std::move(other._mComponentsPerPixel);

		other._mID = 0;
	}

	Texture& Texture::operator = (Texture && other) {
		if(this == &other) {
			return *this;
		}

		_mID                 = std::move(other._mID);
		_mPath               = std::move(other._mPath);
		_mName               = std::move(other._mName);

		_mSource             = std::move(other._mSource);
		_mWrap               = std::move(other._mWrap);
		_mFilter             = std::move(other._mFilter);

		_mData               = std::move(other._mData);
		_mWidth              = std::move(other._mWidth);
		_mHeight             = std::move(other._mHeight);
		_mComponentsPerPixel = std::move(other._mComponentsPerPixel);

		other._mID = 0;

		return *this;
	}

	void Texture::bind(uint32_t slot, uint32_t type) const {
		MY_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
		MY_GL_CHECK(glBindTexture(type, _mID));
	};

	void Texture::unbind() const {
		MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	};

	unsigned int Texture::getID() const {
		return _mID;
	}

	const std::string& Texture::getPath() const {
		return _mPath;
	}

	const std::string& Texture::getName() const {
		return _mName;
	}

	unsigned char* Texture::getData() const {
		bind();

		unsigned char* data = nullptr;

		// Read the pixel data from the texture
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data);

		return data;
	}

	unsigned int Texture::getWidth() const {
		return _mHeight;
	}

	unsigned int Texture::getHeight() const {
		return _mWidth;
	}


	void Texture::clear() {
		if(_mID != 0) {
			MY_GL_CHECK(glDeleteTextures(1, &_mID));
			_mID = 0;
		}
	}

	bool Texture::update() {
		return initData();
	}

	bool Texture::update(TextureWrap wrap) {
		_mWrap = wrap;

		return initData();
	}

	bool Texture::update(TextureFilter filter) {
		_mFilter = filter;

		return initData();
	}

	bool Texture::update(const std::string& file) {
		_mPath = file;
		_mSource = TextureSource::FILE;

		return initData();
	}

	bool Texture::update(const std::string& name, void* data, int width, int height) {
		_mName   = name;
		_mSource = TextureSource::RAW;

		_mData   = static_cast<unsigned char*>(data);
		_mWidth  = width;
		_mHeight = height;

		return initData();
	}

	bool Texture::initData() {
		if (_mSource == TextureSource::FILE) {
			if (_mPath.empty()) {
				printf("[WARN:CORE] Empty texture path!\n");
				return false;
			}
			setName();
		}
		else {
			if(!_mData) {
				_mData = getData();
			}
		}

		if(!setFlip()) {
			return false;
		}

		if (!_mData) {
			// Loading the texture from the specified path
			_mData = stbi_load(_mPath.c_str(), &_mWidth, &_mHeight, &_mComponentsPerPixel, _mBytesPerPixel);
		}

		// Check if the texture buffer is empty (texture load failed)
		if (!_mData) {
			printf("[WARN:CORE] Failed to load texture '%s'. Error: %s\n", _mPath.c_str(), stbi_failure_reason());

			_mData               = nullptr;
			_mWidth              = 0;
			_mHeight             = 0;
			_mComponentsPerPixel = 0;

			return false;
		}

		bind();

		MY_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _mWidth, _mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _mData));
		glGenerateMipmap(GL_TEXTURE_2D);

		// Free the image data that is no longer needed
		stbi_image_free(_mData);
		_mData = nullptr;

		if (!setTextureParams()) {
			return false;
		}
		return true;
	}

	void Texture::setName() {
#ifdef _WIN32
			_mName = _mPath.substr(_mPath.find_last_of('\\') + 1);
#else
			_mName = _mPath.substr(_mPath.find_last_of('/') + 1);
#endif
			_mName = _mName.substr(0, _mName.find_last_of('.'));
	}

	bool Texture::setFlip() const {
		// Flip the texture vertically (OpenGL starts textures from the bottom)
		if(_mSource == TextureSource::FILE) {
			stbi_set_flip_vertically_on_load(true);
		}
		else if(_mSource == TextureSource::RAW) {
			stbi__vertical_flip(_mData, _mWidth, _mHeight, sizeof(float) * 3);
		}
		else {
			printf("[WARN:CORE] Failed to load texture '%s'. Texture source is set to NONE\n", _mPath.c_str());
			return false;
		}
		return true;
	}

	bool Texture::setTextureParams() const {
		unsigned int filter = -1;
		unsigned int wrap   = -1;

		if (_mFilter == TextureFilter::NEAREST) {
			filter = GL_NEAREST;
		}
		else if(_mFilter == TextureFilter::LINEAR) {
			filter = GL_LINEAR;
		}
		else {
			printf("[WARN:CORE] Failed to init texture data. Texture Filter set to NONE\n");
			return false;
		}

		if(_mWrap == TextureWrap::REPEAT) {
			wrap = GL_REPEAT;
		}
		else if(_mWrap == TextureWrap::MIRRORED_REPEAT) {
			wrap = GL_MIRRORED_REPEAT;
		}
		else if(_mWrap == TextureWrap::CALMP_TO_EDGE) {
			wrap = GL_CLAMP_TO_EDGE;
		}
		else if(_mWrap == TextureWrap::CLAMP_TO_BORDER) {
			wrap = GL_CLAMP_TO_BORDER;
		}
		else {
			printf("[WARN:CORE] Failed to init texture data. Texture Wrap set to NONE\n");
			return false;
		}

		// Set texture parameters for minification and magnification filters, and wrapping behavior
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap)); // GL_TEXTURE_WRAP_S: horizontal
		MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap)); // GL_TEXTURE_WRAP_T: vertical

		return true;
	}
};
