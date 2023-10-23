#pragma once

// To avoid including the full glad implementation, we redefine GL_UNIFORM_BUFFER
#ifndef GL_UNIFORM_BUFFER
	#define GL_UNIFORM_BUFFER 0x8A11
#endif

namespace Core {
	class UniformBuffer {
		public:
			UniformBuffer() = delete;
			~UniformBuffer();

			UniformBuffer(const void* data, unsigned int size);

			UniformBuffer(const UniformBuffer& other);
			UniformBuffer& operator = (const UniformBuffer& other);

			UniformBuffer(UniformBuffer && other);
			UniformBuffer& operator = (UniformBuffer && other);
			
			void bind(unsigned int targetBuffer = GL_UNIFORM_BUFFER) const;
			void unbind() const;

			unsigned int getID() const;

		private:
			unsigned int _mID;
			unsigned int _mSize;
	};
};
