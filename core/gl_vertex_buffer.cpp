#include "gl_vertex_buffer.h"

#include "gl_error_handle.h"

#include "error_handle.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size) : _mID(0) {

	MY_GL_CHECK(glGenBuffers(1, &_mID));

	M_ASSERT(_mID != 0);

	MY_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _mID));
	MY_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() {
	MY_GL_CHECK(glDeleteBuffers(1, &_mID));
}

void VertexBuffer::bind() const {
	MY_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, _mID));
}

void VertexBuffer::unbind() const {
	MY_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

unsigned int VertexBuffer::getID() const { return _mID; }
