#pragma once

#include <stdint.h>

class VertexBuffer {
	public:
		/**
		 * @brief Construct a new Vertex Buffer object
		 * 
		 */
		VertexBuffer() = default;

		/**
		 * @brief Construct a new Vertex Buffer object
		 * 
		 * @note https://docs.gl/es3/glBufferData
		 * 
		 * @param[in] data 
		 * @param[in] size 
		 */
		VertexBuffer(const void* data, unsigned int size);

		/**
		 * @brief Destroy the Vertex Buffer object
		 * 
		 * @note https://docs.gl/es3/glDeleteBuffers
		 * 
		 */
		~VertexBuffer();
		
		/**
		 * @brief Binds the VertexBuffer object to the specified buffer binding point
		 * 
		 * @note https://docs.gl/gl3/glBindBuffer
		 */
		void bind() const;

		/**
		 * @brief Binds the VertexBuffer object to the zero binding point
		 * 
		 * @note https://docs.gl/gl3/glBindBuffer
		 * 
		 */
		void unbind() const;

		/**
		 * @brief Get the ID of the object
		 * 
		 * @return unsigned int 
		 */
		unsigned int getID() const;

	private:
		unsigned int _mID;
};
