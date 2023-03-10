#pragma once

#include <stdint.h>
#include <memory.h>

#include "error/gl_error_handle.h"

#include "gl_vertex_array.h"
#include "gl_index_buffer.h"
#include "gl_shader.h"

#define CubeVerticesSize 24
#define CubeIndicesSize  36
#define QuadVerticesSize 20
#define QuadIndicesSize  6

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

		void drawCube(
			const Shader &shader,
			unsigned int drawType = GL_TRIANGLES
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

		std::shared_ptr<VertexArray>  mQuadVA;
		std::shared_ptr<VertexBuffer> mQuadVB;
		std::shared_ptr<IndexBuffer>  mQuadIB;
		std::shared_ptr<VertexBufferLayout> mQuadLayout;

		std::shared_ptr<VertexArray>  mCubeVA;
		std::shared_ptr<VertexBuffer> mCubeVB;
		std::shared_ptr<IndexBuffer>  mCubeIB;
		std::shared_ptr<VertexBufferLayout> mCubeLayout;

		const float mCubeVertices[CubeVerticesSize] {
			-1, -1,  1, //0
			 1, -1,  1, //1
			-1,  1,  1, //2
			 1,  1,  1, //3
			-1, -1, -1, //4
			 1, -1, -1, //5
			-1,  1, -1, //6
			 1,  1, -1  //7
		};
		const uint32_t mCubeIndices[CubeIndicesSize] = {
			//Top
			2, 6, 7,
			2, 3, 7,

			//Bottom
			0, 4, 5,
			0, 1, 5,

			//Left
			0, 2, 6,
			0, 4, 6,

			//Right
			1, 3, 7,
			1, 5, 7,

			//Front
			0, 2, 3,
			0, 1, 3,

			//Back
			4, 6, 7,
			4, 5, 7
		};

		const float mQuadVertices[QuadVerticesSize] = {
			// positions        // texture Coords
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f
		};

		const uint32_t mQuadIndices[QuadIndicesSize] = {
			0, 1, 2,
			2, 1, 3
		};

		//ImGui members
		float mImguiSpeed      = 1.0f;
		bool mImguiShowDetails = false;

		bool mImguiInSet       = true;

		bool mImguiDisabled    = false;
};
