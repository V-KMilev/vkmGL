#pragma once

#include <stdint.h>

class UniformBuffer {
	public:
		/**
		 * @brief Construct a new Uniform Buffer object
		 * 
		 */
		UniformBuffer() = default;

		/**
		 * @brief Construct a new Uniform Buffer object
		 * 
		 * @note https://docs.gl/es3/glBufferData
		 * 
		 * @param[in] data 
		 * @param[in] size 
		 */
		UniformBuffer(const void* data, unsigned int size);

		/**
		 * @brief Destroy the Uniform Buffer object
		 * 
		 * @note https://docs.gl/es3/glDeleteBuffers
		 * 
		 */
		~UniformBuffer();
		
		/**
		 * @brief Binds the UniformBuffer object to the specified buffer binding point
		 * 
		 * @note https://docs.gl/gl3/glBindBuffer
		 */
		void bind() const;

		/**
		 * @brief Binds the UniformBuffer object to the zero binding point
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
		unsigned int mID;
};
