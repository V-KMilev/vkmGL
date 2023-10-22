#include "gl_render_buffer.h"

#include "gl_error_handle.h"

RenderBuffer::RenderBuffer() : _mID(0) {
	MY_GL_CHECK(glGenRenderbuffers(1, &_mID));
}

RenderBuffer::RenderBuffer(
	uint32_t target,
	uint32_t internalformat,
	uint32_t width,
	uint32_t height
)
 : _mID(0)
{
	glGenRenderbuffers(1, &_mID);

	bind();

	MY_GL_CHECK(
		glRenderbufferStorage(
			target,
			internalformat,
			width,
			height
		);
	);
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



void RenderBuffer::update(
	uint32_t target,
	uint32_t internalformat,
	uint32_t width,
	uint32_t height
) const {
	bind();

	MY_GL_CHECK(
		glRenderbufferStorage(
			target,
			internalformat,
			width,
			height
		);
	);
}

uint32_t RenderBuffer::getID() const {
	return _mID;
}
