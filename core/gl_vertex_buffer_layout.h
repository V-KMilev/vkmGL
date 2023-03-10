#pragma once

#include <stdint.h>

#include <vector>

struct VertexBufferElement {

	unsigned int type;           // GLenum
	unsigned int count;          // GLuint
	unsigned char normalized;    // GLboolean
};

class VertexBufferLayout {
	public:
		/**
		 * @brief Construct a new Vertex Buffer Layout object
		 * 
		 */
		VertexBufferLayout();

		/**
		 * @brief Destroy the Vertex Buffer Layout object
		 * 
		 */
		~VertexBufferLayout() = default;

		/**
		 * @brief Pushes T type element to the total elements
		 * 
		 * @note Element is equal to VertexBufferElement. 
		 * We update the stride/count/offset with the given 
		 * count times the T type (in bytes) 
		 * (stride += count * sizeof(T)) 
		 * 
		 * @tparam T 
		 * @param[in] count 
		 */
		template<typename T>
		void push(unsigned int count);

		/**
		 * @brief Get the Elements of the object
		 * 
		 * @return const std::vector<VertexBufferElement>& 
		 */
		const std::vector<VertexBufferElement>& getElements() const;

		/**
		 * @brief Get the Stride of the object
		 * 
		 * @return unsigned int 
		 */
		unsigned int getStride() const;

	private:
		std::vector<VertexBufferElement> myElements;

		unsigned int myStride;
};
