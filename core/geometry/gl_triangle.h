#pragma once

#include "gl_basic_object.h"

// To avoid including the full glad implementation, we redefine GL_TRIANGLES
#ifndef GL_TRIANGLES
	#define GL_TRIANGLES 0x0004
#endif

namespace Core {
	class Triangle : public BasicObject {
		public:
			Triangle();
			~Triangle() = default;

			Triangle(const Triangle& other);
			Triangle& operator = (const Triangle& other);

			Triangle(Triangle && other) = delete;
			Triangle& operator = (Triangle && other) = delete;

			unsigned int getID() const override;

			bool updateTexture(const Texture& texture) override;
			bool updateTexture(unsigned int textureID) override;
			bool updateTexture(const std::string& file) override;

			void draw(const Renderer &renderer, const Shader &shader, unsigned int drawType = GL_TRIANGLES) const override;

		private:
			bool init() override;
			bool deinit() override;

		private:
			static constexpr float _mVertices[TriangleVerticesSize + TextureVertices * TriangleVertices] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 1.0f, 0.0f,    //top left
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,    //bottom left
				 1.0f, -1.0f, 0.0f, 0.0f, 1.0f     //bottom right
			};

			static constexpr unsigned int _mIndices[TriangleIndicesSize] = {
				0, 1, 2
			};
	};
};
