#pragma once

// To avoid including the full glad implementation, we redefine GL_ARRAY_BUFFER
#ifndef GL_ARRAY_BUFFER
	#define GL_ARRAY_BUFFER 0x8892
#endif

namespace Core {
	class VertexBuffer {
		public:
			VertexBuffer() = delete;
			~VertexBuffer();

			VertexBuffer(const void* data, unsigned int size);

			VertexBuffer(const VertexBuffer& other);
			VertexBuffer& operator = (const VertexBuffer& other);

			VertexBuffer(VertexBuffer && other);
			VertexBuffer& operator = (VertexBuffer && other);

			void bind(unsigned int targetBuffer = GL_ARRAY_BUFFER) const;
			void unbind() const;

			unsigned int getID() const;

		private:
			unsigned int _mID;
			unsigned int _mSize;
	};
};
