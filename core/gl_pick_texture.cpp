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
			GL_RGB32F,
			_mWidth,
			_mHeight,
			0,
			GL_RGB,
			GL_FLOAT,
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
			pickParams,
			TextureWrap::CALMP_TO_EDGE,
			TextureFilter::LINEAR
		);

		_mDepthTexture = std::make_shared<Texture>(
			"Depth Texture",
			depthParams,
			TextureWrap::CALMP_TO_EDGE,
			TextureFilter::LINEAR
		);

		FrameBufferParams FBCParams(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			_mPickTexture->getID(),
			0,
			GL_TEXTURE_2D
		);

		FrameBufferParams FBDParams(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			_mDepthTexture->getID(),
			0,
			GL_TEXTURE_2D
		);

		_mFB = std::make_shared<FrameBuffer>(Dimension::_2D);

		_mFB->bind();

		_mPickTexture->bind(_mPickTexture->getID());
		_mPickTexture->update();
		_mFB->update(FBCParams);

		_mDepthTexture->bind(_mDepthTexture->getID());
		_mDepthTexture->update();
		_mFB->update(FBDParams);

		_mFB->unbind();
	}

	void PickTexture::enableWriting() const {
		_mFB->bind(GL_DRAW_FRAMEBUFFER);
	}

	void PickTexture::disableWriting() const {
		_mFB->unbind(GL_DRAW_FRAMEBUFFER);
	}

	PixelInfo PickTexture::readPixel(unsigned int x, unsigned int y) const {
		_mFB->bind(GL_READ_FRAMEBUFFER);

		_mFB->read(GL_COLOR_ATTACHMENT0);

		PixelInfo pixel;

		MY_GL_CHECK(glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &pixel));

		_mFB->read(GL_NONE);

		_mFB->unbind(GL_READ_FRAMEBUFFER);

		return pixel;
	}
};
