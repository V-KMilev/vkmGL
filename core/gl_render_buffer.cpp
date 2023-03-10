#include "gl_render_buffer.h"

#include "error/gl_error_handle.h"

RenderBuffer::RenderBuffer() : mID(0) {
	MY_GL_CHECK(glGenRenderbuffers(1, &mID));
}

RenderBuffer::RenderBuffer(
	uint32_t target,
	uint32_t internalformat,
	uint32_t width,
	uint32_t height
)
 : mID(0)
{
	glGenRenderbuffers(1, &mID);

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
	MY_GL_CHECK(glDeleteRenderbuffers(1, &mID));
}



void RenderBuffer::bind() const {
	MY_GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, mID));
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
	return mID;
}
