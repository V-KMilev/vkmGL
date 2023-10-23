#pragma once

#include <vector>

namespace Core {
	struct VertexBufferElement {

		unsigned int type;           // GLenum
		unsigned int count;          // GLuint
		unsigned char normalized;    // GLboolean
	};

	class VertexBufferLayout {
		public:
			VertexBufferLayout();
			~VertexBufferLayout() = default;

			VertexBufferLayout(const VertexBufferLayout& other);
			VertexBufferLayout& operator = (const VertexBufferLayout& other);

			VertexBufferLayout(VertexBufferLayout && other);
			VertexBufferLayout& operator = (VertexBufferLayout && other);

			template<typename T>
			void push(unsigned int count);

			unsigned int getStride() const;
			const std::vector<VertexBufferElement>& getElements() const;

		private:
			unsigned int _mStride;

			std::vector<VertexBufferElement> _mElements;
	};
};
