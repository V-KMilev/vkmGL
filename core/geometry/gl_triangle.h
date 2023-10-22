#pragma once

#include "gl_basic_object.h"

namespace Core {
	class Triangle : public BasicObject {
		public:
			Triangle();
			~Triangle() = default;

			Triangle(const Triangle& other);
			Triangle& operator = (const Triangle& other);

			Triangle(Triangle && other);
			Triangle& operator = (Triangle && other);

			bool init(textureID) override;
			bool deinit() override;

		private:
			const float _mVertices[TriangleVerticesSize] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 1.0f, 0.0f,    //top left
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,    //bottom left
				 1.0f, -1.0f, 0.0f, 0.0f, 1.0f     //bottom right
			};

			const uint32_t _mIndices[TriangleIndicesSize] = {
				0, 1, 2
			};
	}
};
