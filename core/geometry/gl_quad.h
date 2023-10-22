#pragma once

#include "gl_basic_object.h"

namespace Core {
	class Quad : public BasicObject {
		public:
			Quad();
			~Quad() = default;

			Quad(const Quad& other);
			Quad& operator = (const Quad& other);

			Quad(Quad && other);
			Quad& operator = (Quad && other);

			bool init(textureID) override;
			bool deinit() override;

		private:
			const float _mVertices[QuadVerticesSize] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,    //top left
				 1.0f,  1.0f, 0.0f, 0.0f, 1.0f,    //top right
				-1.0f, -1.0f, 0.0f, 1.0f, 0.0f,    //bottom left
				 1.0f, -1.0f, 0.0f, 1.0f, 1.0f     //bottom right
			};

			const uint32_t _mIndices[QuadIndicesSize] = {
				0, 1, 2,
				2, 1, 3
			};
	}
};
