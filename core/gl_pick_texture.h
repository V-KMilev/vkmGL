#pragma once

#include <memory>

#include "gl_frame_buffer.h"
#include "gl_texture.h"

namespace Core {
	struct PixelInfo {
		unsigned int objectID = 0;
		unsigned int drawID   = 0;
		unsigned int pirmID   = 0;
	};

	class PickTexture {
		public:
			PickTexture(unsigned int width, unsigned int height);
			PickTexture() = default;

			PickTexture(const PickTexture& other) = delete;
			PickTexture operator = (const PickTexture& other) = delete;

			PickTexture(PickTexture && other) = delete;
			PickTexture operator = (PickTexture && other) = delete;

			void enableWriting() const;

			void disableWriting() const;

			PixelInfo readPixel(unsigned int x, unsigned int y) const;

		private:
			std::shared_ptr<FrameBuffer> _mFB;
			std::shared_ptr<Texture>     _mPickTexture;
			std::shared_ptr<Texture>     _mDepthTexture;

			unsigned int _mWidth  = 0;
			unsigned int _mHeight = 0;
	};
};
