#pragma once

#include <stdint.h>
#include <memory.h>

#include "error/gl_error_handle.h"

#include "gl_vertex_array.h"
#include "gl_index_buffer.h"
#include "gl_shader.h"

class Renderer {
	public:
		/**
		 * @brief Construct a new Renderer object
		 * 
		 */
		Renderer();

		/**
		 * @brief Destroy the Renderer object
		 * 
		 */
		~Renderer() = default;

		/**
		 * @brief Clears the set buffers
		 * 
		 * @note https://docs.gl/es3/glClear
		 */
		void clear() const;

		/**
		 * @brief Clear the color buffers with spesific value
		 * 
		 * @note https://docs.gl/es3/glClearColor
		 * 
		 */
		void clearColor() const;

		/**
		 * @brief Draws/Renders the specified array data
		 * 
		 * @note https://docs.gl/es3/glDrawElements
		 * 
		 * @param vertex_array 
		 * @param index_buffer 
		 * @param shader 
		 * @param drawType 
		 */
		void draw(
			const VertexArray &vertex_array,
			const IndexBuffer &index_buffer,
			const Shader &shader,
			unsigned int drawType = GL_TRIANGLES
		) const;

		/**
		 * @brief Draws/Renders the specified array data
		 * 
		 * @note https://docs.gl/es3/glDrawArrays
		 * 
		 * @param[in] vertex_array 
		 * @param[in] drawType 
		 * @param[in] first 
		 * @param[in] count 
		 */
		void draw(
			const VertexArray &vertex_array,
			const Shader &shader,
			unsigned int drawType,
			unsigned int first,
			unsigned int count
		) const;

		/**
		 * @brief 
		 * 
		 * @param[in] count 
		 * @param[in] buffers 
		 */
		void draw(
			uint32_t count,
			const uint32_t *buffers
		) const;

		void drawQuad(
			const Shader &shader,
			unsigned int drawType = GL_TRIANGLES
		) const;

		/**
		 * @brief 
		 * 
		 */
		bool imguiHandle();

	private:
		glm::vec4 mClearColor;

		//ImGui members
		float mImguiSpeed      = 1.0f;
		bool mImguiShowDetails = false;

		bool mImguiInSet       = true;

		bool mImguiDisabled    = false;
};
