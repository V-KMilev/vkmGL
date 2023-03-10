#include "gl_texture.h"

#include <iostream>

#include "error/error_handle.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNM

/////////////////////////////////////////////////////////////////////////////////////////////////
	/* Constructs */

Texture::Texture()
 : mID(0)
 , mType("")
 , mPath("")
 , my_local_buffer(nullptr)
 , width(0)
 , height(0)
 , components_per_pixel(0)
 , mReadType(TextureReadType::DEFAULT)
{}

Texture::Texture(Texture &&texture) noexcept {
	this->mID                  = texture.mID;
	this->mType                = std::move(texture.mType);
	this->mPath                = std::move(texture.mPath);
	this->mReadType            = texture.mReadType;
	this->my_local_buffer      = texture.my_local_buffer;
	this->width                = texture.width;
	this->height               = texture.height;
	this->components_per_pixel = texture.components_per_pixel;

	texture.mID = 0;
	texture.my_local_buffer = nullptr;
}

Texture& Texture::operator = (Texture &&texture) noexcept {
	this->mID                  = texture.mID;
	this->mType                = std::move(texture.mType);
	this->mPath                = std::move(texture.mPath);
	this->mReadType            = texture.mReadType;
	this->my_local_buffer      = texture.my_local_buffer;
	this->width                = texture.width;
	this->height               = texture.height;
	this->components_per_pixel = texture.components_per_pixel;

	texture.mID = 0;
	texture.my_local_buffer = nullptr;

	return *this;
}

Texture::Texture(
	const std::string &texture_path,
	const std::string &type,
	TextureReadType readType,
	unsigned char* data
)
 : mID(0)
 , mPath(texture_path)
 , mType(type)
 , mReadType(readType)
 , my_local_buffer(data)
 , width(0)
 , height(0)
 , components_per_pixel(0)
{

	// OGL starts its textures form the bottom so we need to flip out
	stbi_set_flip_vertically_on_load(true);

	MY_GL_CHECK(glGenTextures(1, &mID));

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, mID));

	if(readType == TextureReadType::DEFAULT) {

		if(!my_local_buffer) {
			my_local_buffer = stbi_load(&texture_path[0], &width, &height, &components_per_pixel, bytes_per_pixel);
		}

		if(!my_local_buffer) {
			std::cerr << "TEXTURE WARNING: FAiled to load DEFAULT Texture image file [" << texture_path
					<< "]\nMessage: " << stbi_failure_reason() << "\n";
			width  = 0;
			height = 0;
		}

		// Can with GL_RGBA and GL_RGBA8
		MY_GL_CHECK(
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA8,
				width, height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				my_local_buffer
			)
		);

		if(my_local_buffer) {
			stbi_image_free(my_local_buffer);
		}
	}
	else if(readType == TextureReadType::HDR) {

		float* my_local_buffer_f;

		my_local_buffer_f = stbi_loadf(&texture_path[0], &width, &height, &components_per_pixel, bytes_per_pixel);

		if(!my_local_buffer_f) {
			std::cerr << "TEXTURE WARNING: FAiled to load HDR Texture image file [" << texture_path
					<< "]\nMessage: " << stbi_failure_reason() << "\n";
			width  = 0;
			height = 0;
		}

		MY_GL_CHECK(
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB16F,
				width, height,
				0,
				GL_RGBA,
				GL_FLOAT,
				my_local_buffer_f
			)
		);

		if(my_local_buffer_f) {
			stbi_image_free(my_local_buffer_f);
		}
	}

	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_S: horizontal
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_T: vertical

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(
	const std::vector<std::string> &texture_paths,
	TextureReadType readType
)
 : mID(0)
 , mPath("")
 , mType("")
 , mReadType(readType)
 , my_local_buffer(nullptr)
 , width(0)
 , height(0)
 , components_per_pixel(0)
{

	// OGL starts its textures form the bottom so we need to flip out
	stbi_set_flip_vertically_on_load(true);

	if(mID == 0) {
		MY_GL_CHECK(glGenTextures(1, &mID));
	}

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mID));

	for(int i = 0; i < 6; i++) {

		if(mReadType == TextureReadType::DEFAULT) {

			my_local_buffer = stbi_load(&texture_paths[i][0], &width, &height, &components_per_pixel, bytes_per_pixel);

			if(!my_local_buffer) {
				std::cerr << "TEXTURE WARNING: Failed to load DEFAULT Texture image file [" << texture_paths[i]
						<< "]\nMessage: " << stbi_failure_reason() << "\n";
				width  = 0;
				height = 0;
			}

			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_RGBA8,
					width, height,
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					my_local_buffer
				)
			);

			if(my_local_buffer) {
			stbi_image_free(my_local_buffer);
			}
		}
		else if(mReadType == TextureReadType::HDR) {

			float* my_local_buffer_f;

			my_local_buffer_f = stbi_loadf(&texture_paths[i][0], &width, &height, &components_per_pixel, bytes_per_pixel);

			if(!my_local_buffer_f) {
				std::cerr << "TEXTURE WARNING: Failed to load HDR Texture image file [" << texture_paths[i]
						<< "]\nMessage: " << stbi_failure_reason() << "\n";
				width  = 0;
				height = 0;
			}

			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_RGB16F,
					width, height,
					0,
					GL_RGBA,
					GL_FLOAT,
					my_local_buffer_f
				)
			);

			if(my_local_buffer_f) {
				stbi_image_free(my_local_buffer_f);
			}
		}
	}

	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::Texture(
	unsigned int target,
	unsigned int level,
	unsigned int internalFormat,
	unsigned int width,
	unsigned int height,
	unsigned int border,
	unsigned int format,
	unsigned int type,
	void*        data,
	TextureReadType readType
)
 : mID(0)
 , mPath("")
 , mType("")
 , mReadType(readType)
 , my_local_buffer(nullptr)
 , width(0)
 , height(0)
 , components_per_pixel(0)
{
	// OGL starts its textures form the bottom so we need to flip out
	// Set to flip the data properly (vertically not horizontally)
	if(data) {
		stbi__vertical_flip(data, width, height, sizeof(float) * 3);
	}

	MY_GL_CHECK(glGenTextures(1, &mID));

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(target, mID));

	if(target == GL_TEXTURE_CUBE_MAP) {

		for(int i = 0; i < 6; i++) {
			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					level,
					internalFormat,
					width, height,
					border,
					format,
					type,
					data
				)
			);
		}

		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	} else {

		MY_GL_CHECK(glTexImage2D(target, level, internalFormat, width, height, border, format, type, data));

		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_S: horizontal
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_T: vertical
	}

	MY_GL_CHECK(glBindTexture(target, 0));
}

Texture::Texture(
	unsigned int target,
	unsigned int level,
	unsigned int internalFormat,
	unsigned int width,
	unsigned int height,
	unsigned int border,
	unsigned int format,
	unsigned int type,
	const std::vector<void*> &data,
	TextureReadType readType
)
 : mID(0)
 , mPath("")
 , mType("")
 , mReadType(readType)
 , my_local_buffer(nullptr)
 , width(0)
 , height(0)
 , components_per_pixel(0)
{
	MY_GL_CHECK(glGenTextures(1, &mID));

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(target, mID));

	if(target != GL_TEXTURE_CUBE_MAP) {

		for(int i = 0; i < 6; i++) {
			// OGL starts its textures form the bottom so we need to flip out
			// Set to flip the data properly (vertically not horizontally)
			if(data[i]) {
				stbi__vertical_flip(data[i], width, height, sizeof(float) * 3);
			}

			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					level,
					internalFormat,
					width, height,
					border,
					format,
					type,
					data[i]
				)
			);
		}
	} else {

		std::cerr << "TEXTURE WARNING: Failed to laod texture target'" << target << "'!\n";
		std::cerr << "TEXTURE WARNING: Texture target needs to be '" << GL_TEXTURE_CUBE_MAP << "'!\n";
	}

	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	MY_GL_CHECK(glBindTexture(target, 0));
}

Texture::~Texture() {
	if(mID != 0) { MY_GL_CHECK(glDeleteTextures(1, &mID)); }
}

void Texture::remove() {
	if(mID != 0) {
		MY_GL_CHECK(glDeleteTextures(1, &mID));
		mID = 0;
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////
	/* Update functions */

void Texture::update(
	const std::string &texture_path,
	TextureReadType readType
) {
	// OGL starts its textures form the bottom so we need to flip out
	stbi_set_flip_vertically_on_load(true);

	if(mID == 0) {
		MY_GL_CHECK(glGenTextures(1, &mID));
	}

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, mID));

	if(readType == TextureReadType::DEFAULT) {

		if(!my_local_buffer) {
			my_local_buffer = stbi_load(&texture_path[0], &width, &height, &components_per_pixel, bytes_per_pixel);
		}

		if(!my_local_buffer) {
			std::cerr << "TEXTURE WARNING: Failed to load DEFAULT Texture image file [" << texture_path
					<< "]\nMessage: " << stbi_failure_reason() << "\n";
			width  = 0;
			height = 0;
		}

		// Can with GL_RGBA and GL_RGBA8
		MY_GL_CHECK(
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA8,
				width, height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				my_local_buffer
			)
		);

		if(my_local_buffer) {
			stbi_image_free(my_local_buffer);
		}
	}
	else if(readType == TextureReadType::HDR) {

		float* my_local_buffer_f;

		my_local_buffer_f = stbi_loadf(&texture_path[0], &width, &height, &components_per_pixel, bytes_per_pixel);

		if(!my_local_buffer_f) {
			std::cerr << "TEXTURE WARNING: FAiled to load HDR Texture image file [" << texture_path
					<< "]\nMessage: " << stbi_failure_reason() << "\n";
			width  = 0;
			height = 0;
		}

		MY_GL_CHECK(
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB16F,
				width, height,
				0,
				GL_RGBA,
				GL_FLOAT,
				my_local_buffer_f
			)
		);

		if(my_local_buffer_f) {
			stbi_image_free(my_local_buffer_f);
		}
	}

	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_S: horizontal
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_T: vertical

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::update(
	const std::vector<std::string> &texture_paths,
	TextureReadType readType
) {
	if(mID == 0) {
		MY_GL_CHECK(glGenTextures(1, &mID));
	}

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mID));

	float* my_local_buffer_f;

	for(int i = 0; i < 6; i++) {

		if(mReadType == TextureReadType::DEFAULT) {

			my_local_buffer = stbi_load(&texture_paths[i][0], &width, &height, &components_per_pixel, bytes_per_pixel);

			if(!my_local_buffer) {
				std::cerr << "TEXTURE WARNING: Failed to load DEFAULT Texture image file [" << texture_paths[i]
						<< "]\nMessage: " << stbi_failure_reason() << "\n";
				width  = 0;
				height = 0;
			}

			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_RGBA8,
					width, height,
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					my_local_buffer
				)
			);

			if(my_local_buffer) {
			stbi_image_free(my_local_buffer);
			}
		}
		else if(mReadType == TextureReadType::HDR) {

			my_local_buffer_f = stbi_loadf(&texture_paths[i][0], &width, &height, &components_per_pixel, bytes_per_pixel);

			if(!my_local_buffer_f) {
				std::cerr << "TEXTURE WARNING: Failed to load HDR Texture image file [" << texture_paths[i]
						<< "]\nMessage: " << stbi_failure_reason() << "\n";
				width  = 0;
				height = 0;
			}

			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_RGB16F,
					width, height,
					0,
					GL_RGBA,
					GL_FLOAT,
					my_local_buffer_f
				)
			);

			if(my_local_buffer_f) {
				stbi_image_free(my_local_buffer_f);
			}
		}
	}

	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::update(
	unsigned int target,
	unsigned int level,
	unsigned int internalFormat,
	unsigned int width,
	unsigned int height,
	unsigned int border,
	unsigned int format,
	unsigned int type,
	void*  data,
	TextureReadType readType
) {
	// OGL starts its textures form the bottom so we need to flip out
	// Set to flip the data properly (vertically not horizontally)
	if(data) {
		stbi__vertical_flip(data, width, height, sizeof(float) * 3);
	}

	if(mID == 0) {
		MY_GL_CHECK(glGenTextures(1, &mID));
	}

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(target, mID));

	if(target == GL_TEXTURE_CUBE_MAP) {

		for(int i = 0; i < 6; i++) {
			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					level,
					internalFormat,
					width, height,
					border,
					format,
					type,
					data
				)
			);
		}

		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	} else {

		MY_GL_CHECK(glTexImage2D(target, level, internalFormat, width, height, border, format, type, data));

		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_S: horizontal
		MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));    // GL_TEXTURE_WRAP_T: vertical
	}

	MY_GL_CHECK(glBindTexture(target, 0));
}

void Texture::update(
	unsigned int target,
	unsigned int level,
	unsigned int internalFormat,
	unsigned int width,
	unsigned int height,
	unsigned int border,
	unsigned int format,
	unsigned int type,
	const std::vector<void*> &data,
	TextureReadType readType
) {
	MY_GL_CHECK(glGenTextures(1, &mID));

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindTexture(target, mID));

	if(target != GL_TEXTURE_CUBE_MAP) {

		for(int i = 0; i < 6; i++) {
			// OGL starts its textures form the bottom so we need to flip out
			// Set to flip the data properly (vertically not horizontally)
			if(data[i]) {
				stbi__vertical_flip(data[i], width, height, sizeof(float) * 3);
			}

			MY_GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					level,
					internalFormat,
					width, height,
					border,
					format,
					type,
					data[i]
				)
			);
		}
	} else {

		std::cerr << "TEXTURE WARNING: Failed to laod texture target'" << target << "'!\n";
		std::cerr << "TEXTURE WARNING: Texture target needs to be '" << GL_TEXTURE_CUBE_MAP << "'!\n";
	}

	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	MY_GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	MY_GL_CHECK(glBindTexture(target, 0));
}



/////////////////////////////////////////////////////////////////////////////////////////////////
	/* Bind / Unbind */

void Texture::bind(uint32_t slot, uint32_t type) const {
	MY_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
	MY_GL_CHECK(glBindTexture(type, mID));
};

void Texture::unbind() const {
	MY_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
};



/////////////////////////////////////////////////////////////////////////////////////////////////
	/* Get functions */

const std::string& Texture::getPath() const {
	return mPath;
}

const std::string& Texture::getType() const {
	return mType;
}
