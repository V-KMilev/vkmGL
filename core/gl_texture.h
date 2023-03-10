#pragma once

#include <stdint.h>

#include <vector>
#include <string>

#include "error/gl_error_handle.h"

enum class TextureReadType {
	DEFAULT = 0,
	HDR     = 1
};

class Texture {
	public:
		/**
		 * @brief Construct a new Texture object
		 * 
		 */
		Texture();

		/**
		 * @brief Copy construcor is BLOCKED
		 * 
		 * @param[in] texture
		 */
		Texture(const Texture& texture) = delete;

		/**
		 * @brief Copy construcor is BLOCKED
		 * 
		 * @param[in] texture 
		 * @return Texture& 
		 */
		Texture& operator = (const Texture& texture) = delete;

		/**
		 * @brief Move constuctor
		 * 
		 * @param[in] texture 
		 */
		Texture(Texture&& texture) noexcept;

		/**
		 * @brief Move constuctor
		 * 
		 * @param[in] texture 
		 * @return Texture& 
		 */
		Texture& operator = (Texture&& texture) noexcept;

		/**
		 * @brief Construct a new Texture object
		 * 
		 * @note https://docs.gl/gl3/glTexImage2D
		 * 
		 * @param[in] file_name 
		 * @param[in] type 
		 * @param[in] readType 
		 * @param[in] data 
		 */
		Texture(
			const std::string &file_name,
			const std::string &type = "",
			TextureReadType readType = TextureReadType::DEFAULT,
			unsigned char* data = nullptr
		);

		/**
		 * @brief Construct a new Texture object
		 * 
		 * @note https://docs.gl/gl3/glTexImage2D
		 * 
		 * @param file_names 
		 */
		Texture(
			const std::vector<std::string> &file_names,
			TextureReadType readType = TextureReadType::DEFAULT
		);

		/**
		 * @brief Construct a new Texture object
		 * 
		 * @note https://docs.gl/gl3/glTexImage2D
		 * 
		 * @param[in] target 
		 * @param[in] level 
		 * @param[in] internalFormat 
		 * @param[in] width 
		 * @param[in] height 
		 * @param[in] border 
		 * @param[in] format 
		 * @param[in] type 
		 * @param[in] data 
		 * @param[in] readType 
		 */
		Texture(
			unsigned int target,
			unsigned int level,
			unsigned int internalFormat,
			unsigned int width,
			unsigned int height,
			unsigned int border,
			unsigned int format,
			unsigned int type,
			void*        data,
			TextureReadType readType = TextureReadType::DEFAULT
		);

		/**
		 * @brief Construct a new Texture:: Texture object
		 * 
		 * @param[in] target 
		 * @param[in] level 
		 * @param[in] internalFormat 
		 * @param[in] width 
		 * @param[in] height 
		 * @param[in] border 
		 * @param[in] format 
		 * @param[in] type 
		 * @param[in] data 
		 * @param[in] readType 
		 */
		Texture::Texture(
			unsigned int target,
			unsigned int level,
			unsigned int internalFormat,
			unsigned int width,
			unsigned int height,
			unsigned int border,
			unsigned int format,
			unsigned int type,
			const std::vector<void*> &data,
			TextureReadType readType = TextureReadType::DEFAULT
		);

		/**
		 * @brief Destroy the Texture object
		 * 
		 * @note https://docs.gl/gl3/glDeleteTextures
		 * 
		 */
		~Texture();

		/**
		 * @brief Removes the Texture object from the GPU side
		 * 
		 * @note https://docs.gl/gl3/glDeleteTextures
		 * 
		 */
		void remove();

		/**
		 * @brief Update/generate of the Texture (if it is not Update/generate)
		 * 
		 * @note  https://docs.gl/gl3/glTexImage2D
		 * 
		 * @param[in] file_name 
		 * @param[in] readType 
		 */
		void update(
			const std::string &file_name,
			TextureReadType readType = TextureReadType::DEFAULT
			);

				/**
		 * @brief Update/generate of the Texture (if it is not Update/generate)
		 * 
		 * @note  https://docs.gl/gl3/glTexImage2D
		 * 
		 * @param[in] file_names 
		 * @param[in] readType 
		 */
		void update(
			const std::vector<std::string> &file_names,
			TextureReadType readType = TextureReadType::DEFAULT
		);

		/**
		 * @brief Update of the Texture's data
		 * 
		 * @note  https://docs.gl/gl3/glTexImage2D
		 * 
		 * @param[in] target 
		 * @param[in] level 
		 * @param[in] internalFormat 
		 * @param[in] width 
		 * @param[in] height 
		 * @param[in] border 
		 * @param[in] format 
		 * @param[in] type 
		 * @param[in] data 
		 * @param[in] readType 
		 */
		void update(
			unsigned int target,
			unsigned int level,
			unsigned int internalFormat,
			unsigned int width,
			unsigned int height,
			unsigned int border,
			unsigned int format,
			unsigned int type,
			void*        data,
			TextureReadType readType = TextureReadType::DEFAULT
		);

		/**
		 * @brief Update of the Texture's data
		 * 
		 * @note  https://docs.gl/gl3/glTexImage2D
		 * 
		 * @param[in] target 
		 * @param[in] level 
		 * @param[in] internalFormat 
		 * @param[in] width 
		 * @param[in] height 
		 * @param[in] border 
		 * @param[in] format 
		 * @param[in] type 
		 * @param[in] data 
		 * @param[in] readType 
		 */
		void update(
			unsigned int target,
			unsigned int level,
			unsigned int internalFormat,
			unsigned int width,
			unsigned int height,
			unsigned int border,
			unsigned int format,
			unsigned int type,
			const std::vector<void*> &data,
			TextureReadType readType = TextureReadType::DEFAULT
		);

		/**
		 * @brief Binds the Texture object to the specified texturing binding point
		 * 
		 * @note https://docs.gl/gl3/glBindTexture
		 * https://docs.gl/gl3/glActiveTexture
		 * 
		 * @param[in] slot 
		 * @param[in] type 
		 */
		void bind(uint32_t slot = 0, uint32_t type = GL_TEXTURE_2D) const;

		/**
		 * @brief Binds the Texture object to the zero binding point
		 * 
		 * @note https://docs.gl/gl3/glBindTexture
		 * 
		 */
		void unbind() const;

		const std::string& getPath() const;

		const std::string& getType() const;

	private:
		std::string mType;
		std::string mPath;
		std::string mName;

		TextureReadType mReadType;

		unsigned char* my_local_buffer;

		static const int bytes_per_pixel = 4;
		int width, height;
		int components_per_pixel;

		unsigned int mID;
};
