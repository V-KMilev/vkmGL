#include "gl_vertex_array.h"

#include <utility>

#include "gl_error_handle.h"
#include "l_assert.h"

namespace Core {
	VertexArray::VertexArray() : _mID(0) {
		VKM_GL_CHECK(glGenVertexArrays(1, &_mID));

		VKM_ASSERT(_mID != 0);
	}

	VertexArray::~VertexArray() {
		VKM_GL_CHECK(glDeleteVertexArrays(1, &_mID));
	}

	void VertexArray::bind() const {
		VKM_GL_CHECK(glBindVertexArray(_mID));
	}

	void VertexArray::unbind() const {
		VKM_GL_CHECK(glBindVertexArray(0));
	}

	unsigned int VertexArray::getID() const {
		return _mID;
	}

	void VertexArray::addBuffer(const VertexBuffer &vertex_buffer, const VertexBufferLayout& layout) const {
		bind();
		vertex_buffer.bind();

		const std::vector<VertexBufferElement>& elements = layout.getElements();

		unsigned int offset = 0;

		for (unsigned int idx = 0; idx < elements.size(); idx++) {
			const VertexBufferElement &element = elements[idx];

			VKM_GL_CHECK(glEnableVertexAttribArray(idx));
			VKM_GL_CHECK(glVertexAttribPointer(idx, element.count, element.type, element.normalized, layout.getStride(), (const void*) offset));

			/* The offset equals to the count of the previous element times the size of the element type */
			offset += element.count * sizeof(element.type);
		}
	}
};
