#pragma once

// To avoid including the full glad implementation, we redefine GL_FRAMEBUFFER
#ifndef GL_FRAMEBUFFER
	#define GL_FRAMEBUFFER 0x8D40
#endif

namespace Core {
	enum class Dimensions {
		_ND  = 0,
		_1D  = 1,
		_2D  = 2,
		_3D  = 3
	};

	class FrameBuffer {
		public:
			FrameBuffer(Dimensions dimensions);
			~FrameBuffer();

			FrameBuffer(const FrameBuffer& other) = delete;
			FrameBuffer& operator = (const FrameBuffer& other) = delete;

			FrameBuffer(FrameBuffer && other) = delete;
			FrameBuffer& operator = (FrameBuffer && other) = delete;

			void bind(unsigned int targetBuffer = GL_FRAMEBUFFER) const;
			void unbind() const;

			unsigned int getID() const;

			bool init(
				unsigned int target,
				unsigned int attachment,
				unsigned int textureID,
				unsigned int level,
				unsigned int textarget = 0,
				unsigned int layer = 0
			) const;

			bool update(
				unsigned int target,
				unsigned int attachment,
				unsigned int textureID,
				unsigned int level,
				unsigned int textarget = 0,
				unsigned int layer = 0
			) const;

			void bindRenderBuffer(
				unsigned int target,
				unsigned int attachment,
				unsigned int renderbuffertarget,
				unsigned int renderbufferID
			) const;


		private:
			unsigned int _mID;

			Dimensions _mDimensions;
	};
};
