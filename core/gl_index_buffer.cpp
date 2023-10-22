#include "gl_index_buffer.h"

#include "gl_error_handle.h"

#include "error_handle.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : _mID(0), mCount(count) {

	MY_GL_CHECK(glGenBuffers(1, &_mID));

	M_ASSERT(_mID != 0);

	MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mID));
	MY_GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer() {
	MY_GL_CHECK(glDeleteBuffers(1, &_mID));
}

void IndexBuffer::bind() const {
	MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mID));
}

void IndexBuffer::unbind() const {
	MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

unsigned int IndexBuffer::getCount() const { return mCount; }

unsigned int IndexBuffer::getID() const { return _mID; }
