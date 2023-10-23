#pragma once

namespace Core {
	class RenderBuffer {
		public:
			RenderBuffer();
			~RenderBuffer();

			RenderBuffer(
				unsigned int target,
				unsigned int internalformat,
				unsigned int width,
				unsigned int height
			);

			RenderBuffer(const RenderBuffer& other) = delete;
			RenderBuffer& operator = (const RenderBuffer& other) = delete;

			RenderBuffer(RenderBuffer && other) = delete;
			RenderBuffer& operator = (RenderBuffer && other) = delete;

			void bind() const;
			void unbind() const;

			unsigned int getID() const;

			bool init(
				unsigned int target,
				unsigned int internalformat,
				unsigned int width,
				unsigned int height
			) const;

			bool update(
				unsigned int target,
				unsigned int internalformat,
				unsigned int width,
				unsigned int height
			) const;


		private:
			unsigned int _mID;
	};
};
