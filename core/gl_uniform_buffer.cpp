#include "gl_uniform_buffer.h"

#include "gl_error_handle.h"

#include "error_handle.h"

UniformBuffer::UniformBuffer(const void* data, unsigned int size) : _mID(0) {

	MY_GL_CHECK(glGenBuffers(1, &_mID));

	M_ASSERT(_mID != 0);

	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, _mID));
	MY_GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW));
	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

UniformBuffer::~UniformBuffer() {
	MY_GL_CHECK(glDeleteBuffers(1, &_mID));
}

void UniformBuffer::bind() const {
	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, _mID));
}

void UniformBuffer::unbind() const {
	MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

unsigned int UniformBuffer::getID() const { return _mID; }
