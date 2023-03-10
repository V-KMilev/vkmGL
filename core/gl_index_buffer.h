#pragma once

#include <stdint.h>

class IndexBuffer {
	public:
		/**
		 * @brief Delete construcor is BLOCKED
		 * 
		 */
		IndexBuffer() = delete;

		/**
		 * @brief Construct a new Index Buffer object
		 * 
		 * @note https://docs.gl/es3/glBufferData
		 * 
		 * @param[in] data 
		 * @param[in] count 
		 */
		IndexBuffer(const unsigned int* data, unsigned int count);

		/**
		 * @brief Destroy the Index Buffer object
		 * 
		 * @note https://docs.gl/es3/glDeleteBuffers
		 * 
		 */
		~IndexBuffer();

		/**
		 * @brief Binds the IndexBuffer object to the specified buffer binding point
		 * 
		 * @note https://docs.gl/gl3/glBindBuffer
		 */
		void bind() const;

		/**
		 * @brief Binds the IndexBuffer object to the zero binding point
		 * 
		 * @note https://docs.gl/gl3/glBindBuffer
		 * 
		 */
		void unbind() const;

		/**
		 * @brief Get the Count of the object
		 * 
		 * @return unsigned int 
		 */
		unsigned int getCount() const;

		/**
		 * @brief Get the ID of the object
		 * 
		 * @return unsigned int 
		 */
		unsigned int getID() const;

	private:
		unsigned int mID;
		unsigned int mCount;
};
