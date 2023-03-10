#include "gl_vertex_buffer.h"

#include "error/gl_error_handle.h"

#include "error/error_handle.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size) : mID(0) {

	MY_GL_CHECK(glGenBuffers(1, &mID));

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mID));
	MY_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() {
	MY_GL_CHECK(glDeleteBuffers(1, &mID));
}

void VertexBuffer::bind() const {
	MY_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mID));
}

void VertexBuffer::unbind() const {
	MY_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

unsigned int VertexBuffer::getID() const { return mID; }
