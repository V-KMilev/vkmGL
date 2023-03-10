#include "gl_vertex_buffer_layout.h"

#include <assert.h>

#include "error/gl_error_handle.h"

#include "error/error_handle.h"

VertexBufferLayout::VertexBufferLayout() : myStride(0) {}

template<typename T>
void push(unsigned int count) {

	M_ASSERT(false);
}

const std::vector<VertexBufferElement>& VertexBufferLayout::getElements() const { return myElements; }

unsigned int VertexBufferLayout::getStride() const { return myStride; }

template<>
void VertexBufferLayout::push<float>(unsigned int count) {

	VertexBufferElement element;
	element.type = GL_FLOAT;
	element.count = count;
	element.normalized = GL_FALSE;

	myElements.push_back(element);

	/* Custom for every template : float == GLfloat */
	myStride += count * sizeof(GLfloat);
}
