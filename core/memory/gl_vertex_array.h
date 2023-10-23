#pragma once

#include "gl_vertex_buffer_layout.h"
#include "gl_vertex_buffer.h"

namespace Core {
	class VertexArray {
		public:
			VertexArray();
			~VertexArray();

			VertexArray(const VertexArray& other) = delete;
			VertexArray& operator = (const VertexArray& other) = delete;

			VertexArray(VertexArray && other) = delete;
			VertexArray& operator = (VertexArray && other) = delete;

			void bind() const;
			void unbind() const;

			unsigned int getID() const;

			void addBuffer(const VertexBuffer& vertex_buffer, const VertexBufferLayout& layout) const;

		private:
			unsigned int _mID;
	};
};
