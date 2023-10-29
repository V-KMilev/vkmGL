#include "gl_pick_texture.h"

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	PickTexture::PickTexture(unsigned int width, unsigned int height) : 
		_mWidth(width),
		_mHeight(height)
	{
		TextureParams pickParams(
			GL_TEXTURE_2D,
			0,
			GL_RGB32UI,
			_mWidth,
			_mHeight,
			0,
			GL_RGB_INTEGER,
			GL_UNSIGNED_INT,
			nullptr
		);

		TextureParams depthParams(
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT,
			_mWidth,
			_mHeight,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			nullptr
		);

		_mPickTexture = std::make_shared<Texture>(
			"Pick Texture",
			TextureWrap::CLAMP_TO_BORDER,
			TextureFilter::NEAREST,
			pickParams
		);

		_mDepthTexture = std::make_shared<Texture>(
			"Depth Texture",
			TextureWrap::CLAMP_TO_BORDER,
			TextureFilter::NEAREST,
			depthParams
		);

		_mFB = std::make_shared<FrameBuffer>(Dimension::_2D);

		FrameBufferParams FBCParams(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D,
			_mPickTexture->getID(),
			0
		);

		FrameBufferParams FBDParams(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D,
			_mPickTexture->getID(),
			0
		);

		_mFB->bind();

		_mPickTexture->init();
		_mPickTexture->bind();
		_mFB->update(FBCParams);

		_mDepthTexture->init();
		_mDepthTexture->bind();
		_mFB->update(FBDParams);

		_mFB->unbind();
		_mPickTexture->unbind();
		_mDepthTexture->unbind();
	}

	void PickTexture::enableWriting() const {
		_mFB->bind(GL_DRAW_FRAMEBUFFER);
	}

	void PickTexture::disableWriting() const {
		_mFB->unbind(GL_DRAW_FRAMEBUFFER);
	}

	PixelInfo PickTexture::readPixel(unsigned int x, unsigned int y) const {
		_mFB->bind(GL_READ_FRAMEBUFFER);

		glReadBuffer(GL_COLOR_ATTACHMENT0);

		PixelInfo pixel;
		glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &pixel);

		glReadBuffer(GL_NONE);

		_mFB->unbind(GL_READ_FRAMEBUFFER);

		return pixel;
	}
};
