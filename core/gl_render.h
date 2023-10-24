#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/rotate_vector.hpp"

#include "gl_vertex_array.h"
#include "gl_index_buffer.h"
#include "gl_shader.h"

namespace Core {
	class Renderer {
		public:
			Renderer();
			~Renderer() = default;

			Renderer(const Renderer& other);
			Renderer& operator = (const Renderer& other);

			Renderer(Renderer && other);
			Renderer& operator = (Renderer && other);

			void clear() const;
			void clearColor() const;

			void draw(
				const VertexArray& vertex_array,
				const IndexBuffer& index_buffer,
				const Shader &shader,
				unsigned int drawType
			) const;

			void draw(
				const VertexArray& vertex_array,
				const Shader& shader,
				unsigned int drawType,
				unsigned int first,
				unsigned int count
			) const;

			void draw(
				unsigned int count,
				const unsigned int* buffers
			) const;

		private:
			glm::vec4 _mClearColor;
	};
};