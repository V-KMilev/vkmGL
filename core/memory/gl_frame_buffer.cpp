#include "gl_frame_buffer.h"

#include <stdio.h>

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	FrameBuffer::FrameBuffer(Dimensions dimensions) : _mID(0), _mDimensions(dimensions) {
		MY_GL_CHECK(glGenFramebuffers(1, &_mID));
	}

	FrameBuffer::~FrameBuffer() {
		MY_GL_CHECK(glDeleteFramebuffers(1, &_mID));
	}

	void FrameBuffer::bind(unsigned int target) const {
		MY_GL_CHECK(glBindFramebuffer(target, _mID));
	}

	void FrameBuffer::unbind() const {
		MY_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	unsigned int FrameBuffer::getID() const {
		return _mID;
	}

	bool FrameBuffer::init(
		unsigned int target,
		unsigned int attachment,
		unsigned int textureID,
		unsigned int level,
		unsigned int textarget,
		unsigned int layer
	) const {
		bind();

		if (_mDimensions == Dimensions::_ND) {
			MY_GL_CHECK(glFramebufferTexture(target, attachment, textureID, level));
		}
		else if (_mDimensions == Dimensions::_1D) {
			MY_GL_CHECK(glFramebufferTexture1D(target, attachment, textarget, textureID, level));
		}
		else if (_mDimensions == Dimensions::_2D) {
			MY_GL_CHECK(glFramebufferTexture2D(target, attachment, textarget, textureID, level));
		}
		else if (_mDimensions == Dimensions::_3D) {
			MY_GL_CHECK(glFramebufferTexture3D(target, attachment, textarget, textureID, level, layer));
		}
		else {
			printf("[ERROR:CORE] FBO: [ID:%ud] has invalid dimensions!", _mID);

			return false;
		}

		unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		M_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);

		return true;
	}

	bool FrameBuffer::update(
		unsigned int target,
		unsigned int attachment,
		unsigned int textureID,
		unsigned int level,
		unsigned int textarget,
		unsigned int layer
	) const {
		return init(target, attachment, textureID, level, textarget, layer);
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
