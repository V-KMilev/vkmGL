#pragma once

#include <stdint.h>

#include "gl_vertex_buffer_layout.h"
#include "gl_vertex_buffer.h"

class VertexArray {
	public:

		/**
		 * @brief Construct a new Vertex Array object
		 * 
		 * @note https://docs.gl/gl3/glGenVertexArrays
		 * 
		 */
		VertexArray();

		/**
		 * @brief Destroy the Vertex Array object
		 * 
		 * @note https://docs.gl/gl3/glDeleteVertexArrays
		 * 
		 */
		~VertexArray();

		/**
		 * @brief Binds the VertexArray object to the specified array binding point
		 * 
		 */
		void bind() const;

		/**
		 * @brief Binds the VertexArray object to the zero binding point
		 * 
		 */
		void unbind() const;
		
		/**
		 * @brief Get the ID of the object
		 * 
		 * @return unsigned int 
		 */
		unsigned int getID() const;

		/**
		 * @brief Adds AttribArray to the VertexArray
		 * 
		 * @note We pass two buffers, VertexBuffer and VertexBufferLayout.
		 * We pass the VertexBuffer to bind the vertices we are going to use. 
		 * We pass the VertexBufferLayout to get the layout of our vertices. 
		 * For each element in the layout we secify the corresponding data 
		 * format 
		 * https://docs.gl/es3/glEnableVertexAttribArray
		 * https://docs.gl/es3/glVertexAttribPointer
		 * 
		 * @param[in] vertex_buffer 
		 * @param[in] layout 
		 */
		void addBuffer(const VertexBuffer &vertex_buffer, const VertexBufferLayout& layout);

	private:
		unsigned int mID;
};
