#pragma once

// To avoid including the full glad implementation, we redefine GL_ELEMENT_ARRAY_BUFFER
#ifndef GL_ELEMENT_ARRAY_BUFFER
	#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

namespace Core {
	class IndexBuffer {
	public:
		IndexBuffer() = delete;
		~IndexBuffer();

		IndexBuffer(const unsigned int* data, unsigned int count);

		IndexBuffer(const IndexBuffer& other);
		IndexBuffer& operator = (const IndexBuffer& other);

		IndexBuffer(IndexBuffer && other);
		IndexBuffer& operator = (IndexBuffer && other);

		void bind(unsigned int targetBuffer = GL_ELEMENT_ARRAY_BUFFER) const;
		void unbind() const;

		unsigned int getID() const;
		unsigned int getCount() const;

	private:
		unsigned int _mID;
		unsigned int _mCount;
	};
};
