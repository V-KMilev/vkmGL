#pragma once

#include "gl_basic_object.h"

namespace Core {
	class Triangle : public BasicObject {
		public:
			Triangle(unsigned int textureID);
			~Triangle() = default;

			Triangle(const Triangle& other);
			Triangle& operator = (const Triangle& other);

			Triangle(Triangle && other);
			Triangle& operator = (Triangle && other);

			bool init(unsigned int textureID = 0) override;
			bool deinit() override;

		private:
			const float _mVertices[TriangleVerticesSize + TextureVertices * TriangleVertices] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 1.0f, 0.0f,    //top left
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,    //bottom left
				 1.0f, -1.0f, 0.0f, 0.0f, 1.0f     //bottom right
			};

			const uint32_t _mIndices[TriangleIndicesSize] = {
				0, 1, 2
			};
	};
};
