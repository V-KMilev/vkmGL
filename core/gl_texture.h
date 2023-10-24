#pragma once

#include <string>

// To avoid including the full glad implementation, we redefine GL_TEXTURE_2D
#ifndef GL_TEXTURE_2D
	#define GL_TEXTURE_2D 0x0DE1
#endif

namespace Core {
	enum class TextureSource {
		FILE    = 0,
		RAW     = 1
	};

	class Texture {
		public:
			Texture() = delete;
			~Texture();

			Texture(const std::string& file);
			Texture(const std::string& name, void* data);

			Texture(const Texture& other);
			Texture& operator = (const Texture& other);

			Texture(Texture && other);
			Texture& operator = (Texture && other);

			void bind(unsigned int slot = 0, unsigned int type = GL_TEXTURE_2D) const;
			void unbind() const;

			const std::string& getPath() const;
			const std::string& getName() const;

			unsigned int getID() const;

			void clear();

			bool update();
			bool update(unsigned int textureID);
			bool update(const Texture& texture);
			bool update(const std::string& file);
			bool update(const std::string& name, void* data);

		private:
			bool init();

		private:
			unsigned int _mID;

			std::string _mPath;
			std::string _mName;

			TextureSource _mSource;

			static const int _mBytesPerPixel = 4;

			unsigned char* _mData;
			int _mWidth;
			int _mHeight;
			int _mComponentsPerPixel;
	};
};