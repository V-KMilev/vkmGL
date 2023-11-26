#include "gl_texture.h"

#include <utility>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNM

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	TextureParams::TextureParams(
		unsigned int target,
		unsigned int level,
		unsigned int internalFormat,
		int          width,
		int          height,
		unsigned int border,
		unsigned int format,
		unsigned int type,
		void* data,
		int componentsPerPixel
	) : 
		target(target),
		level(level),
		internalFormat(internalFormat),
		width(width),
		height(height),
		border(border),
		format(format),
		type(type),
		componentsPerPixel(componentsPerPixel)
	{
		if(data) {
			this->data = static_cast<unsigned char*>(data);
		}
	}

	Texture::~Texture() {
		if(_mID != 0) {
			MY_GL_CHECK(glDeleteTextures(1, &_mID));
		}
	}

	Texture::Texture(
		const std::string& name,
		TextureParams      params,
		TextureWrap        wrap,
		TextureFilter      filter
	) :
		_mName(name),
		_mSource(TextureSource::RAW),
		_mParams(params),
		_mWrap(wrap),
		_mFilter(filter)
	{
		// Generate a new texture
		MY_GL_CHECK(glGenTextures(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	Texture::Texture(
		const std::string& name,
		TextureWrap        wrap,
		TextureFilter      filter,
		void* data,
		unsigned int target,
		unsigned int level,
		unsigned int internalFormat,
		unsigned int width,
		unsigned int height,
		unsigned int border,
		unsigned int format,
		unsigned int type
	) :
		_mName(name),
		_mSource(TextureSource::RAW),
		_mWrap(wrap),
		_mFilter(filter),
		_mParams(
			TextureParams(
				target,
				level,
				internalFormat,
				width,
				height,
				border,
				format,
				type,
				data
			)
		)
	{
		// Generate a new texture
		MY_GL_CHECK(glGenTextures(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	Texture::Texture(const std::string& file) :
		_mPath(file),
		_mSource(TextureSource::FILE)
	{
		// Generate a new texture
		MY_GL_CHECK(glGenTextures(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	Texture::Texture(Texture && other) noexcept {
		_mID    = std::move(other._mID);
		_mPath  = std::move(other._mPath);
		_mName  = std::move(other._mName);

		_mSource = std::move(other._mSource);
		_mWrap   = std::move(other._mWrap);
		_mFilter = std::move(other._mFilter);

		_mParams = std::move(other._mParams);

		other._mID   = 0;
		other._mPath.clear();
		other._mName.clear();
	}

	Texture& Texture::operator = (Texture && other) noexcept  {
		if(this == &other) {
			return *this;
		}

		_mID   = std::move(other._mID);
		_mPath = std::move(other._mPath);
		_mName = std::move(other._mName);

		_mSource = std::move(other._mSource);
		_mWrap   = std::move(other._mWrap);
		_mFilter = std::move(other._mFilter);

		_mParams = std::move(other._mParams);

		other._mID   = 0;
		other._mPath = "";
		other._mName = "";

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

	const TextureParams& Texture::getParams() const {
		return _mParams;
	}

	TextureParams& Texture::getParams() {
		return _mParams;
	}

	unsigned char* Texture::getData() const {
		bind();

		unsigned char* data = nullptr;

		// Read the pixel data from the texture
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data);

		return data;
	}

	unsigned int Texture::getWidth() const {
		return _mParams.width;
	}

	unsigned int Texture::getHeight() const {
		return _mParams.height;
	}

	bool Texture::init() {
		if (_mSource == TextureSource::FILE) {
			if (_mPath.empty()) {
				printf("[WARN:CORE] Empty texture path!\n");
				return false;
			}

			setName();

			if(!setFlip()) {
				return false;
			}

			if (!_mParams.data) {
				// Loading the texture from the specified path
				_mParams.data = stbi_load(
					_mPath.c_str(),
					&_mParams.width,
					&_mParams.height,
					&_mParams.componentsPerPixel,
					_mBytesPerPixel
				);
			}

			// Check if the texture buffer is empty (texture load failed)
			if (!_mParams.data) {
				printf("[WARN:CORE] Failed to load texture '%s'. Error: %s\n", _mPath.c_str(), stbi_failure_reason());

				_mParams.data               = nullptr;
				_mParams.width              = 0;
				_mParams.height             = 0;
				_mParams.componentsPerPixel = 0;

				return false;
			}
		}

		bind();

		MY_GL_CHECK(glTexImage2D(
			_mParams.target,
			_mParams.level,
			_mParams.internalFormat,
			_mParams.width,
			_mParams.height,
			_mParams.border,
			_mParams.format,
			_mParams.type,
			_mParams.data
		));

		glGenerateMipmap(GL_TEXTURE_2D);

		if (_mParams.data) {
			// Free the image data that is no longer needed
			stbi_image_free(_mParams.data);
			_mParams.data = nullptr;
		}

		if (!setTextureProperties()) {

			unbind();
			return false;
		}

		unbind();
		return true;
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

	bool Texture::update(TextureWrap wrap) {
		_mWrap = wrap;

		return init();
	}

	bool Texture::update(TextureFilter filter) {
		_mFilter = filter;

		return init();
	}

	bool Texture::update(const std::string& file) {
		_mPath = file;
		_mSource = TextureSource::FILE;

		return init();
	}

	bool Texture::update(const std::string& name, void* data, int width, int height) {
		_mName   = name;
		_mSource = TextureSource::RAW;

		_mParams.data   = static_cast<unsigned char*>(data);
		_mParams.width  = width;
		_mParams.height = height;

		return init();
	}

	void Texture::setName() {
		_mName = _mPath.substr(_mPath.find_last_of('/') + 1);
		_mName = _mName.substr(0, _mName.find_last_of('.'));
	}

	bool Texture::setFlip() const {
		// Flip the texture vertically (OpenGL starts textures from the bottom)
		if(_mSource == TextureSource::FILE) {
			stbi_set_flip_vertically_on_load(true);
		}
		else {
			printf("[WARN:CORE] Failed to load texture '%s'. Texture source is set to NONE\n", _mPath.c_str());
			return false;
		}
		return true;
	}

	bool Texture::setTextureProperties() const {
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
