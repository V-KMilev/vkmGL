#include "gl_vertex_buffer.h"

#include <utility>

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	VertexBuffer::VertexBuffer(const void* data, unsigned int size) : _mID(0), _mSize(size) {
		MY_GL_CHECK(glGenBuffers(1, &_mID));

		// Ensure the buffer ID is valid
		M_ASSERT(_mID != 0);

		bind();

		// Fill the buffer with data
		MY_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, _mSize, data, GL_STATIC_DRAW));
	}

	VertexBuffer::~VertexBuffer() {
		// Delete the OpenGL buffer
		MY_GL_CHECK(glDeleteBuffers(1, &_mID));
	}

	VertexBuffer::VertexBuffer(const VertexBuffer& other) : _mID(0), _mSize(0) {
		_mSize = other._mSize;

		MY_GL_CHECK(glGenBuffers(1, &_mID));

		// Ensure the buffer ID is valid
		M_ASSERT(_mID != 0);

		other.bind(GL_COPY_READ_BUFFER);
		this->bind(GL_COPY_WRITE_BUFFER);

		// Create a new buffer
		MY_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, _mSize, nullptr, GL_STATIC_DRAW));

		// Copy the data from the other's buffer
		MY_GL_CHECK(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _mSize));
	}

	VertexBuffer& VertexBuffer::operator = (const VertexBuffer& other) {
		if (this == &other) {
			return *this;
		}

		_mSize = other._mSize;

		MY_GL_CHECK(glGenBuffers(1, &_mID));

		// Ensure the buffer ID is valid
		M_ASSERT(_mID != 0);

		other.bind(GL_COPY_READ_BUFFER);
		this->bind(GL_COPY_WRITE_BUFFER);

		// Create a new buffer
		MY_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, _mSize, nullptr, GL_STATIC_DRAW));

		// Copy the data from the other's buffer
		MY_GL_CHECK(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _mSize));

		return *this;
	}

	VertexBuffer::VertexBuffer(VertexBuffer && other) : _mID(std::move(other._mID)), _mSize(std::move(other._mSize)) {
		// Reset the moved-from data
		other._mSize = 0;
		other._mID   = 0;
	}

	VertexBuffer& VertexBuffer::operator = (VertexBuffer && other) {
		if (this == &other) {
			return *this;
		}

		this->_mSize = std::move(other._mSize);
		this->_mID   = std::move(other._mID);

		// Reset the moved-from data
		other._mSize = 0;
		other._mID   = 0;

		return *this;
	}

	void VertexBuffer::bind(unsigned int targetBuffer) const {
		MY_GL_CHECK(glBindBuffer(targetBuffer, _mID));
	}

	void VertexBuffer::unbind() const {
		MY_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	unsigned int VertexBuffer::getID() const {
		return _mID;
	}
};
