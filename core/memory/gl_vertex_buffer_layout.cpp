#include "gl_vertex_buffer_layout.h"

#include <utility>

#include "gl_error_handle.h"
#include "error_handle.h"

namespace Core {
	VertexBufferLayout::VertexBufferLayout() :  _mStride(0), _mElements({}) {}

	VertexBufferLayout::VertexBufferLayout(const VertexBufferLayout& other) {
		this->_mElements = other._mElements;
		this->_mStride   = other._mStride;
	}

	VertexBufferLayout& VertexBufferLayout::operator = (const VertexBufferLayout& other) {
		if (this == &other) {
			return *this;
		}

		this->_mElements = other._mElements;
		this->_mStride   = other._mStride;
		return *this;
	}

	VertexBufferLayout::VertexBufferLayout(VertexBufferLayout && other) {
		this->_mElements = std::move(other._mElements);
		this->_mStride   = std::move(other._mStride);
	}

	VertexBufferLayout& VertexBufferLayout::operator = (VertexBufferLayout && other) {
		if (this == &other) {
			return *this;
		}

		this->_mElements = std::move(other._mElements);
		this->_mStride   = std::move(other._mStride);
		return *this;
	}

	template<typename T>
	void push(unsigned int count) {
		// Error message indicating that push function should be used
		M_ASSERT(false);
	}

	unsigned int VertexBufferLayout::getStride() const {
		return _mStride;
	}

	const std::vector<VertexBufferElement>& VertexBufferLayout::getElements() const {
		return _mElements;
	}

	template<>
	void VertexBufferLayout::push<float>(unsigned int count) {
		VertexBufferElement element;
		element.type       = GL_FLOAT;
		element.count      = count;
		element.normalized = GL_FALSE;

		_mElements.push_back(element);

		// Update the stride based on the element's size
		_mStride += count * sizeof(GLfloat);
	}
};
