#pragma once

#include <string>

// To avoid including the full glad implementation, we redefine GL_TEXTURE_2D
#ifndef GL_TEXTURE_2D
	#define GL_TEXTURE_2D 0x0DE1
#endif

namespace Core {
	enum class TextureSource {
		NONE = 0,
		FILE = 1,
		RAW  = 1
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

	class Texture {
		public:
			Texture();
			~Texture();

			Texture(const std::string& file);
			Texture(const std::string& name, void* data);

			Texture(const Texture& other);
			Texture& operator = (const Texture& other);

			Texture(Texture && other);
			Texture& operator = (Texture && other);

			void bind(unsigned int slot = 0, unsigned int type = GL_TEXTURE_2D) const;
			void unbind() const;

			unsigned int       getID() const;

			const std::string& getPath() const;
			const std::string& getName() const;

			unsigned char*     getData() const;
			unsigned int       getWidth() const;
			unsigned int       getHeight() const;

			void clear();

			bool update();
			bool update(TextureWrap wrap);
			bool update(TextureFilter filter);
			bool update(const std::string& file);
			bool update(const std::string& name, void* data, int width, int height);

		private:
			bool initData();

			void setName();

			bool setFlip() const;
			bool setTextureParams() const;

		private:
			unsigned int _mID = 0;

			std::string _mPath = "";
			std::string _mName = "";

			TextureSource _mSource = TextureSource::NONE;
			TextureWrap   _mWrap   = TextureWrap::CALMP_TO_EDGE;
			TextureFilter _mFilter = TextureFilter::LINEAR;

			unsigned char* _mData    = nullptr;
			int _mWidth              = 0;
			int _mHeight             = 0;
			int _mComponentsPerPixel = 0;

			static const int _mBytesPerPixel = 4;
	};
};
