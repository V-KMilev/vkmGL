#include "gl_uniform_buffer.h"

#include <utility>

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	UniformBuffer::UniformBuffer(const void* data, unsigned int size) : _mID(0), _mSize(size) {
		MY_GL_CHECK(glGenBuffers(1, &_mID));

		// Ensure the buffer ID is valid
		M_ASSERT(_mID != 0);

		bind();

		// Fill the buffer with data
		MY_GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, _mSize, data, GL_STATIC_DRAW));
	}

	UniformBuffer::~UniformBuffer() {
		MY_GL_CHECK(glDeleteBuffers(1, &_mID));
	}

	UniformBuffer::UniformBuffer(const UniformBuffer& other) : _mID(0), _mSize(0) {
		_mSize = other._mSize;

		MY_GL_CHECK(glGenBuffers(1, &_mID));

		// Ensure the buffer ID is valid
		M_ASSERT(_mID != 0);

		other.bind(GL_COPY_READ_BUFFER);
		this->bind(GL_COPY_WRITE_BUFFER);

		// Create a new buffer
		MY_GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, _mSize, nullptr, GL_STATIC_DRAW));

		// Copy the data from the other's buffer
		MY_GL_CHECK(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _mSize));
	}

	UniformBuffer& UniformBuffer::operator = (const UniformBuffer& other) {
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
		MY_GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, _mSize, nullptr, GL_STATIC_DRAW));

		// Copy the data from the other's buffer
		MY_GL_CHECK(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _mSize));

		return *this;
	}

	UniformBuffer::UniformBuffer(UniformBuffer && other) {
		this->_mID = std::move(other._mID);
	}

	UniformBuffer& UniformBuffer::operator = (UniformBuffer && other) {
		if (this == &other) {
			return *this;
		}

		this->_mID = std::move(other._mID);
		return *this;
	}

	void UniformBuffer::bind(unsigned int targetBuffer) const {
		MY_GL_CHECK(glBindBuffer(targetBuffer, _mID));
	}

	void UniformBuffer::unbind() const {
		MY_GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}

	unsigned int UniformBuffer::getID() const {
		return _mID;
	}
};
