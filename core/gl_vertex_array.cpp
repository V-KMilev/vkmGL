#include "gl_vertex_array.h"

#include "error/gl_error_handle.h"

#include "error/error_handle.h"

VertexArray::VertexArray() : mID(0) {
	MY_GL_CHECK(glGenVertexArrays(1, &mID));

	M_ASSERT(mID != 0);
}

VertexArray::~VertexArray() {
	MY_GL_CHECK(glDeleteVertexArrays(1, &mID));
}

void VertexArray::bind() const {
	MY_GL_CHECK(glBindVertexArray(mID));
}

void VertexArray::unbind() const {
	MY_GL_CHECK(glBindVertexArray(0));
}

void VertexArray::addBuffer(const VertexBuffer &vertex_buffer, const VertexBufferLayout& layout) {

	bind();
	vertex_buffer.bind();

	const std::vector<VertexBufferElement> &elements = layout.getElements();

	unsigned int offset = 0;

	for (unsigned int position = 0; position < elements.size(); position++) {

		const VertexBufferElement &element = elements[position];

		MY_GL_CHECK(glEnableVertexAttribArray(position));
		MY_GL_CHECK(glVertexAttribPointer(position, element.count, element.type, element.normalized, layout.getStride(), (const void*) offset));

		/* The offset equals to the count of the previous element times the size of the element type */
		offset += element.count * sizeof(element.type);
	}
}

unsigned int VertexArray::getID() const { return mID; }
