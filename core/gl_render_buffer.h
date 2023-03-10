#pragma once

#include <stdint.h>

class RenderBuffer {
	public:
		RenderBuffer();

		RenderBuffer(
			uint32_t target,
			uint32_t internalformat,
			uint32_t width,
			uint32_t height
		);

		~RenderBuffer();

		void bind() const;

		void unbind() const;

		void update(
			uint32_t target,
			uint32_t internalformat,
			uint32_t width,
			uint32_t height
		) const;

		uint32_t getID() const;

	private:
		unsigned int mID;
};
