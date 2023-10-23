#include "gl_render_buffer.h"

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	RenderBuffer::RenderBuffer() : _mID(0) {
		MY_GL_CHECK(glGenRenderbuffers(1, &_mID));
	}

	RenderBuffer::~RenderBuffer() {
		MY_GL_CHECK(glDeleteRenderbuffers(1, &_mID));
	}

	void RenderBuffer::bind() const {
		MY_GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, _mID));
	}

	void RenderBuffer::unbind() const {
		MY_GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
	}

	unsigned int RenderBuffer::getID() const {
		return _mID;
	}

	bool RenderBuffer::init(
		unsigned int target,
		unsigned int internalformat,
		unsigned int width,
		unsigned int height
	) const {
		bind();

		MY_GL_CHECK(glRenderbufferStorage(target, internalformat, width, height));

		return true;
	}

	bool RenderBuffer::update(
		unsigned int target,
		unsigned int internalformat,
		unsigned int width,
		unsigned int height
	) const {
		return init(target, internalformat, width, height);
	}
}
