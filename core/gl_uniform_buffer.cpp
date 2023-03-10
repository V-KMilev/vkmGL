#include "gl_uniform_buffer.h"

#include "error/gl_error_handle.h"

#include "error/error_handle.h"

UniformBuffer::UniformBuffer(const void* data, unsigned int size) : mID(0) {

	MY_GL_CHECK(glGenBuffers(1, &mID));

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, mID));
	MY_GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW));
	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

UniformBuffer::~UniformBuffer() {
	MY_GL_CHECK(glDeleteBuffers(1, &mID));
}

void UniformBuffer::bind() const {
	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, mID));
}

void UniformBuffer::unbind() const {
	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

unsigned int UniformBuffer::getID() const { return mID; }
