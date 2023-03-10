#include "gl_index_buffer.h"

#include "error/gl_error_handle.h"

#include "error/error_handle.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : mID(0), mCount(count) {

	MY_GL_CHECK(glGenBuffers(1, &mID));

	M_ASSERT(mID != 0);

	MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mID));
	MY_GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer() {
	MY_GL_CHECK(glDeleteBuffers(1, &mID));
}

void IndexBuffer::bind() const {
	MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mID));
}

void IndexBuffer::unbind() const {
	MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

unsigned int IndexBuffer::getCount() const { return mCount; }

unsigned int IndexBuffer::getID() const { return mID; }
