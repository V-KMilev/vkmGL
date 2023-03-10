#pragma once

#include <stdint.h>

#include "error/gl_error_handle.h"

class FrameBuffer {
	public:
		FrameBuffer();

		FrameBuffer(
			uint32_t target,
			uint32_t attachment,
			uint32_t textarget,
			uint32_t textureID,
			uint32_t level
		);

			FrameBuffer(
			uint32_t target,
			uint32_t attachment,
			uint32_t textureID,
			uint32_t level
		);

		~FrameBuffer();

		void bind(uint32_t target = GL_FRAMEBUFFER) const;

		void unbind(uint32_t target = GL_FRAMEBUFFER) const;

		void bindRenderBuffer(
			uint32_t target,
			uint32_t attachment,
			uint32_t renderbuffertarget,
			uint32_t renderbufferID
		) const;

		void update(
			uint32_t target,
			uint32_t attachment,
			uint32_t textarget,
			uint32_t textureID,
			uint32_t level
		) const;

		void update(
			uint32_t target,
			uint32_t attachment,
			uint32_t textureID,
			uint32_t level
		) const;

		uint32_t getID() const;

	private:
		unsigned int mID;
};
