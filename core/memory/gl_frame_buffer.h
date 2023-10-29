#pragma once

// To avoid including the full glad implementation, we redefine GL_FRAMEBUFFER
#ifndef GL_FRAMEBUFFER
	#define GL_FRAMEBUFFER 0x8D40
#endif

#ifndef GL_TEXTURE_2D
	#define GL_TEXTURE_2D 0x0DE1
#endif

#ifndef GL_COLOR_ATTACHMENT0
	#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif

namespace Core {
	enum class Dimension {
		_ND  = 0,
		_1D  = 1,
		_2D  = 2,
		_3D  = 3
	};

	struct FrameBufferParams {
		public:
			FrameBufferParams() = default;

			FrameBufferParams(
				unsigned int target,
				unsigned int attachment,
				unsigned int textarget = 0,
				unsigned int textureID = 0,
				unsigned int level     = 0,
				unsigned int layer     = 0
			);

		public:
			unsigned int target     = GL_FRAMEBUFFER;
			unsigned int attachment = GL_COLOR_ATTACHMENT0;
			unsigned int textarget  = GL_TEXTURE_2D;
			unsigned int textureID  = 0;
			unsigned int level      = 0;
			unsigned int layer      = 0;
	};

	class FrameBuffer {
		public:
			FrameBuffer() = delete;
			~FrameBuffer();

			FrameBuffer(
				Dimension dimension,
				FrameBufferParams params = FrameBufferParams()
			);

			FrameBuffer(
				Dimension dimension,
				unsigned int target,
				unsigned int attachment,
				unsigned int textarget = 0,
				unsigned int textureID = 0,
				unsigned int level     = 0,
				unsigned int layer     = 0
			);


			FrameBuffer(const FrameBuffer& other) = delete;
			FrameBuffer& operator = (const FrameBuffer& other) = delete;

			FrameBuffer(FrameBuffer && other) = delete;
			FrameBuffer& operator = (FrameBuffer && other) = delete;

			void bind(unsigned int targetBuffer = GL_FRAMEBUFFER) const;
			void unbind(unsigned int targetBuffer = GL_FRAMEBUFFER) const;

			unsigned int getID() const;

			bool init() const;

			bool update(
				FrameBufferParams params
			);

			bool update(
				unsigned int target,
				unsigned int attachment,
				unsigned int texture,
				unsigned int level,
				unsigned int textarget = 0,
				unsigned int layer = 0
			);

			void bindRenderBuffer(
				unsigned int target,
				unsigned int attachment,
				unsigned int renderbuffertarget,
				unsigned int renderbufferID
			) const;

		private:
			unsigned int _mID = 0;

			Dimension _mDimensions;

			FrameBufferParams _mParams;
	};
};
