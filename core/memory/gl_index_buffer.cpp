#include "gl_index_buffer.h"

#include <utility>

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : _mID(0), _mCount(count) {
		MY_GL_CHECK(glGenBuffers(1, &_mID));

		M_ASSERT(_mID != 0);

		bind();

		MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mID));
		// Fill the buffer with data
		MY_GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mCount * sizeof(unsigned int), data, GL_STATIC_DRAW));
	}

	IndexBuffer::IndexBuffer(const IndexBuffer& other) : _mID(0), _mCount(other._mCount) {
		_mCount = other._mCount;

		MY_GL_CHECK(glGenBuffers(1, &_mID));

		// Ensure the buffer ID is valid
		M_ASSERT(_mID != 0);

		other.bind(GL_COPY_READ_BUFFER);
		this->bind(GL_COPY_WRITE_BUFFER);

		// Create a new buffer
		MY_GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mCount * sizeof(unsigned int), nullptr, GL_STATIC_DRAW));

		// Copy the data from the other's buffer
		MY_GL_CHECK(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _mCount * sizeof(unsigned int)));
	}

	IndexBuffer& IndexBuffer::operator = (const IndexBuffer& other) {
		if (this == &other) {
			return *this;
		}

		_mCount = other._mCount;

		MY_GL_CHECK(glGenBuffers(1, &_mID));

		// Ensure the buffer ID is valid
		M_ASSERT(_mID != 0);

		other.bind(GL_COPY_READ_BUFFER);
		this->bind(GL_COPY_WRITE_BUFFER);

		// Create a new buffer
		MY_GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mCount * sizeof(unsigned int), nullptr, GL_STATIC_DRAW));

		// Copy the data from the other's buffer
		MY_GL_CHECK(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _mCount * sizeof(unsigned int)));

		return *this;
	}

	IndexBuffer::IndexBuffer(IndexBuffer && other) : _mID(std::move(other._mID)), _mCount(std::move(other._mCount)) {
		// Reset the moved-from data
		other._mID    = 0;
		other._mCount = 0;
	}

	IndexBuffer& IndexBuffer::operator = (IndexBuffer && other) {
		if (this == &other) {
			return *this;
		}

		_mID    = std::move(other._mID);
		_mCount = std::move(other._mCount);

		other._mID    = 0;
		other._mCount = 0;

		return *this;
	}

	IndexBuffer::~IndexBuffer() {
		MY_GL_CHECK(glDeleteBuffers(1, &_mID));
	}

	void IndexBuffer::bind(unsigned int targetBuffer) const {
		MY_GL_CHECK(glBindBuffer(targetBuffer, _mID));
	}

	void IndexBuffer::unbind() const {
		MY_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}

	unsigned int IndexBuffer::getID() const {
		return _mID;
	}

	unsigned int IndexBuffer::getCount() const {
		return _mCount;
	}

}
