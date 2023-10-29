#include "gl_frame_buffer.h"

#include <stdio.h>

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	FrameBufferParams::FrameBufferParams(
		unsigned int target,
		unsigned int attachment,
		unsigned int textarget,
		unsigned int textureID,
		unsigned int level,
		unsigned int layer
	) : 
		target(target),
		attachment(attachment),
		textarget(textarget),
		textureID(textureID),
		level(level),
		layer(layer) {}

	FrameBuffer::~FrameBuffer() {
		MY_GL_CHECK(glDeleteFramebuffers(1, &_mID));
	}

	FrameBuffer::FrameBuffer(Dimension dimension, FrameBufferParams params) :
		_mDimensions(dimension),
		_mParams(params)
	{
		// Generate a new Frame Buffer
		MY_GL_CHECK(glGenFramebuffers(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	FrameBuffer::FrameBuffer(
		Dimension dimension,
		unsigned int target,
		unsigned int attachment,
		unsigned int textarget,
		unsigned int texture,
		unsigned int level,
		unsigned int layer
	) :
		_mDimensions(dimension),
		_mParams(
			FrameBufferParams(
			target,
			attachment,
			textarget,
			texture,
			level,
			layer
			)
		)
	{
		// Generate a new Frame Buffer
		MY_GL_CHECK(glGenFramebuffers(1, &_mID));

		M_ASSERT(_mID != 0);
	}

	void FrameBuffer::bind(unsigned int target) const {
		MY_GL_CHECK(glBindFramebuffer(target, _mID));
	}

	void FrameBuffer::unbind(unsigned int target) const {
		MY_GL_CHECK(glBindFramebuffer(target, 0));
	}

	unsigned int FrameBuffer::getID() const {
		return _mID;
	}

	bool FrameBuffer::init(
	) const {
		bind();

		if (_mDimensions == Dimension::_ND) {
			MY_GL_CHECK(glFramebufferTexture(
				_mParams.target,
				_mParams.attachment,
				_mParams.textureID,
				_mParams.level
			));
		}
		else if (_mDimensions == Dimension::_1D) {
			MY_GL_CHECK(glFramebufferTexture1D(
				_mParams.target,
				_mParams.attachment,
				_mParams.textarget,
				_mParams.textureID,
				_mParams.level
			));
		}
		else if (_mDimensions == Dimension::_2D) {
			MY_GL_CHECK(glFramebufferTexture2D(
				_mParams.target,
				_mParams.attachment,
				_mParams.textarget,
				_mParams.textureID,
				_mParams.level
			));
		}
		else if (_mDimensions == Dimension::_3D) {
			MY_GL_CHECK(glFramebufferTexture3D(
				_mParams.target,
				_mParams.attachment,
				_mParams.textarget,
				_mParams.textureID,
				_mParams.level,
				_mParams.layer
			));
		}
		else {
			printf("[ERROR:CORE] FBO: [ID:%ud] has invalid dimension!", _mID);
			unbind();

			return false;
		}

		unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		M_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);

		unbind();

		return true;
	}

	bool FrameBuffer::update(
		FrameBufferParams params
	) {
		_mParams = params;

		return init();
	}

	bool FrameBuffer::update(
		unsigned int target,
		unsigned int attachment,
		unsigned int textureID,
		unsigned int level,
		unsigned int textarget,
		unsigned int layer
	) {
		_mParams = FrameBufferParams(
			target,
			attachment,
			textureID,
			level,
			textarget,
			layer
		);

		return init();
	}

	void FrameBuffer::bindRenderBuffer(
		unsigned int target,
		unsigned int attachment,
		unsigned int renderbuffertarget,
		unsigned int renderbufferID
	) const {
		bind();

		MY_GL_CHECK(glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbufferID));

		unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		M_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
	}
};
