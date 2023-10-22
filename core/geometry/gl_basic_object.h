#pragma once

#include <stdint.h>
#include <memory.h>

#include "gl_vertex_array.h"
#include "gl_vertex_buffer.h"
#include "gl_index_buffer.h"
#include "gl_vertex_buffer_layout.h"

#include "object_id.h"

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

	class BasicObject {
	public:
		virtual ~BasicObject() = default;

		virtual BasicObject(const BasicObject& other) = 0;
		virtual BasicObject& operator = (const BasicObject& other) = 0;

		virtual BasicObject(BasicObject && other) = 0;
		virtual BasicObject& operator = (BasicObject && other) = 0;

		virtual bool init(textureID = 0) = 0;
		virtual bool deinit() = 0;

	private:
		std::shared_ptr<VertexArray>        _mVA;
		std::shared_ptr<VertexBuffer>       _mVB;
		std::shared_ptr<IndexBuffer>        _mIB;
		std::shared_ptr<VertexBufferLayout> _mVBL;

		std::shared_ptr<Texture> _mTexture;

		Common::ObjectID _mID;
	};
}