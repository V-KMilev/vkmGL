#include "gl_frame_buffer.h"

#include <stdint.h>

#include "error_handle.h"

#include "gl_texture.h"

FrameBuffer::FrameBuffer() : _mID(0) {
	MY_GL_CHECK(glGenFramebuffers(1, &_mID));
}

FrameBuffer::FrameBuffer(
	uint32_t target,
	uint32_t attachment,
	uint32_t textarget,
	uint32_t textureID,
	uint32_t level
)
 : _mID(0)
{
	MY_GL_CHECK(glGenFramebuffers(1, &_mID));

	bind();

	MY_GL_CHECK(
		glFramebufferTexture2D(
			target,
			attachment,
			textarget,
			textureID,
			level
		)
	);

	uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	M_ASSERT(
		status == GL_FRAMEBUFFER_COMPLETE,
		std::string("FBO ERROR: " + std::to_string(status) + " | FBO #" + std::to_string(_mID) + " not complete!").data()
	);
}

FrameBuffer::FrameBuffer(
	uint32_t target,
	uint32_t attachment,
	uint32_t textureID,
	uint32_t level
)
 : _mID(0)
{
	MY_GL_CHECK(glGenFramebuffers(1, &_mID));

	bind();

	MY_GL_CHECK(
		glFramebufferTexture(
			target,
			attachment,
			textureID,
			level
		)
	);

	uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	M_ASSERT(
		status == GL_FRAMEBUFFER_COMPLETE,
		std::string("FBO ERROR: " + std::to_string(status) + " | FBO #" + std::to_string(_mID) + " not complete!").data()
	);
}

FrameBuffer::~FrameBuffer() {
	MY_GL_CHECK(glDeleteFramebuffers(1, &_mID));
}



void FrameBuffer::bind(uint32_t target) const {
	MY_GL_CHECK(glBindFramebuffer(target, _mID));
}

void FrameBuffer::unbind(uint32_t target) const {
	MY_GL_CHECK(glBindFramebuffer(target, 0));
}



void FrameBuffer::bindRenderBuffer(
	uint32_t target,
	uint32_t attachment,
	uint32_t renderbuffertarget,
	uint32_t renderbufferID
) const {
	bind();

	MY_GL_CHECK(
		glFramebufferRenderbuffer(
			target,
			attachment,
			renderbuffertarget,
			renderbufferID
		)
	);

	uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	M_ASSERT(
		status == GL_FRAMEBUFFER_COMPLETE,
		std::string("FBO ERROR: " + std::to_string(status) + " | FBO #" + std::to_string(_mID) + " not complete!").data()
	);
}

void FrameBuffer::update(
	uint32_t target,
	uint32_t attachment,
	uint32_t textarget,
	uint32_t textureID,
	uint32_t level
) const {
	bind();

	MY_GL_CHECK(
		glFramebufferTexture2D(
			target,
			attachment,
			textarget,
			textureID,
			level
		)
	);

	uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	M_ASSERT(
		status == GL_FRAMEBUFFER_COMPLETE,
		std::string("FBO ERROR: " + std::to_string(status) + " | FBO [" + std::to_string(_mID) + "] not complete!").data()
	);
}

void FrameBuffer::update(
	uint32_t target,
	uint32_t attachment,
	uint32_t textureID,
	uint32_t level
) const {
	bind();

	MY_GL_CHECK(
		glFramebufferTexture(
			target,
			attachment,
			textureID,
			level
		)
	);

	uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	M_ASSERT(
		status == GL_FRAMEBUFFER_COMPLETE,
		std::string("FBO ERROR: " + std::to_string(status) + " | FBO [" + std::to_string(_mID) + "] not complete!").data()
	);
}

uint32_t FrameBuffer::getID() const {
	return _mID;
}
