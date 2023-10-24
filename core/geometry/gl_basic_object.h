#pragma once

#include <memory>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/rotate_vector.hpp"

#include "gl_render.h"
#include "gl_shader.h"
#include "gl_texture.h"

#include "gl_vertex_array.h"
#include "gl_vertex_buffer.h"
#include "gl_index_buffer.h"
#include "gl_vertex_buffer_layout.h"

#include "object_id.h"

// To avoid including the full glad implementation, we redefine GL_TRIANGLES
#ifndef GL_TRIANGLES
	#define GL_TRIANGLES 0x0004
#endif

namespace Core {
	// Basic define primitive geometry
	#define Dimensions 3

	#define TextureVertices  2
	#define TriangleVertices 3
	#define QuadVertices     4

	#define TriangleVerticesSize Dimensions * TriangleVertices
	#define QuadVerticesSize     Dimensions * QuadVertices

	#define TriangleIndices 3
	#define QuadIndices     6

	#define TriangleIndicesSize TriangleIndices
	#define QuadIndicesSize     QuadIndices

struct ObjectWorldData {
	// x, y, z
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale    = glm::vec3(1.0f, 1.0f, 1.0f);

	// roll, pitch, yaw
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
};

class BasicObject {
	public:
		BasicObject();
		virtual ~BasicObject() = default;

		BasicObject(const BasicObject& other);
		BasicObject& operator = (const BasicObject& other);

		BasicObject(BasicObject&& other) = delete;
		BasicObject& operator = (BasicObject&& other) = delete;

		virtual bool updateTexture(const Texture& texture)  = 0;
		virtual bool updateTexture(unsigned int textureID)  = 0;
		virtual bool updateTexture(const std::string& file) = 0;

		virtual void draw(const Renderer &renderer, const Shader &shader, unsigned int drawType = GL_TRIANGLES) const = 0;

		virtual unsigned int getID() const = 0;

	protected:
		virtual bool init() = 0;
		virtual bool deinit() = 0;

	protected:
		Common::ObjectID _mID;

		ObjectWorldData _mWorldData;

		std::shared_ptr<Texture> _mTexture;

		std::shared_ptr<VertexArray>        _mVA;
		std::shared_ptr<VertexBuffer>       _mVB;
		std::shared_ptr<IndexBuffer>        _mIB;
		std::shared_ptr<VertexBufferLayout> _mVBL;
	};
}
