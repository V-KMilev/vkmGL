#pragma once

#include <string>

// To avoid including the full glad implementation, we redefine GL_TEXTURE_2D
#ifndef GL_TEXTURE_2D
	#define GL_TEXTURE_2D 0x0DE1
#endif

#ifndef GL_RGBA8
	#define GL_RGBA8 0x8058
#endif

#ifndef GL_RGBA
	#define GL_RGBA 0x1908
#endif

#ifndef GL_UNSIGNED_BYTE
	#define GL_UNSIGNED_BYTE 0x1401
#endif

namespace Core {
	enum class TextureSource {
		NONE     = 0,
		FILE     = 1,
		RAW      = 2
	};

	enum class TextureWrap {
		NONE            = 0,
		REPEAT          = 1,
		MIRRORED_REPEAT = 2,
		CALMP_TO_EDGE   = 3,
		CLAMP_TO_BORDER = 4
	};

	enum class TextureFilter {
		NONE    = 0,
		NEAREST = 1,
		LINEAR  = 2
	};

	struct TextureParams {
		public:
			TextureParams() = default;

			TextureParams(
				unsigned int target,
				unsigned int level,
				unsigned int internalFormat,
				int          width,
				int          height,
				unsigned int border,
				unsigned int format,
				unsigned int type,
				void* data,
				int componentsPerPixel = 0
			);

		public:
			unsigned int target         = GL_TEXTURE_2D;
			unsigned int level          = 0;
			unsigned int internalFormat = GL_RGBA8;
			int          width          = 0;
			int          height         = 0;
			unsigned int border         = 0;
			unsigned int format         = GL_RGBA;
			unsigned int type           = GL_UNSIGNED_BYTE;

		public:
			unsigned char* data    = nullptr;
			int componentsPerPixel = 0;
	};

	class Texture {
		public:
			Texture() = delete;
			~Texture();

			Texture(
				const std::string& name,
				TextureParams      params,
				TextureWrap        wrap   = TextureWrap::CALMP_TO_EDGE,
				TextureFilter      filter = TextureFilter::LINEAR
			);

			Texture(
				const std::string& name,
				TextureWrap        wrap,
				TextureFilter      filter,
				void* data                  = nullptr,
				unsigned int target         = GL_TEXTURE_2D,
				unsigned int level          = 0,
				unsigned int internalFormat = GL_RGBA8,
				unsigned int width          = 0,
				unsigned int height         = 0,
				unsigned int border         = 0,
				unsigned int format         = GL_RGBA,
				unsigned int type           = GL_UNSIGNED_BYTE
			);

			Texture(const std::string& file);

			Texture(const Texture& other) = delete;
			Texture& operator = (const Texture& other) = delete;

			Texture(Texture && other) noexcept;
			Texture& operator = (Texture && other) noexcept;

			void bind(unsigned int slot = 0, unsigned int type = GL_TEXTURE_2D) const;
			void unbind() const;

			unsigned int getID() const;

			const std::string& getPath() const;
			const std::string& getName() const;

			const TextureParams& getParams() const;
			TextureParams& getParams();

			unsigned char*     getData() const;
			unsigned int       getWidth() const;
			unsigned int       getHeight() const;

			bool init();

			void clear();

			bool update();
			bool update(TextureWrap wrap);
			bool update(TextureFilter filter);
			bool update(const std::string& file);
			bool update(const std::string& name, void* data, int width, int height);

		private:
			void setName();

			bool setFlip() const;
			bool setTextureProperties() const;

		private:
			unsigned int _mID = 0;

			std::string _mPath = "";
			std::string _mName = "";

			TextureSource _mSource = TextureSource::NONE;
			TextureWrap   _mWrap   = TextureWrap::CALMP_TO_EDGE;
			TextureFilter _mFilter = TextureFilter::LINEAR;

			TextureParams _mParams;

			static const int _mBytesPerPixel = 4;
	};
};
