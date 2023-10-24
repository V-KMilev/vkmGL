#pragma once

#include "gl_basic_object.h"

namespace Core {
	class Quad : public BasicObject {
		public:
			Quad();
			~Quad() = default;

			Quad(const Quad& other);
			Quad& operator = (const Quad& other);

			Quad(Quad && other) = delete;
			Quad& operator = (Quad && other) = delete;

			unsigned int getID() const override;

			bool updateTexture(const Texture& texture) override;
			bool updateTexture(unsigned int textureID) override;
			bool updateTexture(const std::string& file) override;

			void draw(const Renderer &renderer, const Shader &shader, unsigned int drawType) const override;

		private:
			bool init() override;
			bool deinit() override;
		private:
			const float _mVertices[QuadVerticesSize + TextureVertices * QuadVertices] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,    //top left
				 1.0f,  1.0f, 0.0f, 0.0f, 1.0f,    //top right
				-1.0f, -1.0f, 0.0f, 1.0f, 0.0f,    //bottom left
				 1.0f, -1.0f, 0.0f, 1.0f, 1.0f     //bottom right
			};

			const unsigned int _mIndices[QuadIndicesSize] = {
				0, 1, 2,
				2, 1, 3
			};
	};
};
