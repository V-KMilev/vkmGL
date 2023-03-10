#pragma once

#include <stdint.h>

#include <unordered_map>
#include <string>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/rotate_vector.hpp"

struct ProgramShaders {
	/**
	 * @brief Construct a new Program Shaders object
	 * 
	 * @param[in] vertexShader 
	 * @param[in] fragmentShader 
	 * @param[in] geometryShader 
	 */
	ProgramShaders(
		const std::string &vertexShader,
		const std::string &fragmentShader,
		const std::string &geometryShader
	);

	const std::string vertexShader;
	const std::string fragmentShader;
	const std::string geometryShader;
};

class Shader {
	public:
		/**
		 * @brief Construct a new Shader object
		 * 
		 * @param[in] vertexFileName 
		 * @param[in] fragmentFileName 
		 * @param[in] geometryFileName 
		 */
		Shader(
			const std::string &vertexFileName,
			const std::string &fragmentFileName,
			const std::string &geometryFileName = ""
		);

		Shader(const Shader& shader) = delete;

		Shader& operator = (const Shader& shader) = delete;

		/**
		 * @brief Destroy the Shader/Program object
		 * 
		 * @note https://docs.gl/es3/glDeleteProgram
		 * 
		 */
		~Shader();

		/**
		 * @brief Installs a Shader/Program object to the specified buffer binding point
		 * 
		 * @note https://docs.gl/es3/glUseProgram
		 * 
		 */
		void bind() const;

		/**
		 * @brief Installs the Shader/Program object to the zero binding point
		 * 
		 * @note https://docs.gl/es3/glUseProgram
		 * 
		 */
		void unbind() const;

		/**
		 * @brief Set the Uniform4f object
		 * 
		 * @note https://docs.gl/es3/glUniform
		 * 
		 * @param[in] uniformName 
		 * @param[in] f0 
		 * @param[in] f1 
		 * @param[in] f2 
		 * @param[in] f3 
		 */
		void setUniform4f(
			const std::string &uniformName,
			float f0,
			float f1,
			float f2,
			float f3
		) const;

		/**
		 * @brief Set the Uniform3f object
		 * 
		 * @note https://docs.gl/es3/glUniform
		 * 
		 * @param[in] uniformName 
		 * @param[in] f0 
		 * @param[in] f1 
		 * @param[in] f2 
		 */
		void setUniform3f(
			const std::string &uniformName,
			float f0,
			float f1,
			float f2
		) const;

		/**
		 * @brief Set the Uniform1f object
		 * 
		 * @note https://docs.gl/es3/glUniform
		 * 
		 * @param[in] uniformName 
		 * @param[in] f0 
		 */
		void setUniform1f(const std::string &uniformName, float f0) const;

		/**
		 * @brief Set the Uniform3fv object
		 * 
		 * @note https://docs.gl/es3/glUniform
		 * 
		 * @param[in] uniformName 
		 * @param[in] v0 
		 */
		void setUniform3fv(const std::string &uniformName, const glm::vec3& v0) const;

		/**
		 * @brief Set the Uniform Matrix4fv object
		 * 
		 * @note https://docs.gl/es3/glUniform
		 * 
		 * @param[in] uniformName 
		 * @param[in] matrix 
		 */
		void setUniformMatrix4fv(const std::string &uniformName, const glm::mat4 &matrix) const;

		/**
		 * @brief Set the Uniform1i object
		 * 
		 * @note https://docs.gl/es3/glUniform
		 * 
		 * @param[in] uniformName 
		 * @param[in] i0 
		 */
		void setUniform1i(const std::string &uniformName, int i0) const;

		/**
		 * @brief Set the Uniform1i object
		 * 
		 * @note https://docs.gl/es3/glUniform
		 * 
		 * @param[in] uniformName 
		 * @param[in] ui0 
		 */
		void setUniform1ui(const std::string &uniformName, uint32_t ui0) const;

		/**
		 * @brief 
		 * 
		 */
		void reCompleShader();

		/**
		 * @brief Get the Path object
		 * 
		 * @return std::string 
		 */
		const std::string& getPath() const;

		/**
		 * @brief Get the Update Flag object
		 * 
		 * @return true 
		 * @return false 
		 */
		bool getUpdateFlag() const;

		/**
		 * @brief Set the Update Flag object
		 * 
		 * @param value 
		 */
		void setUpdateFlag(bool value);

	private:
		/**
		 * @brief Get the Uniform Location object
		 * 
		 * @note https://docs.gl/es3/glGetUniformLocation 
		 * There is a small optimisation with a repetitions 
		 * of uniforms. If there is already existing uniform 
		 * with the current set it will return the location if it 
		 * 
		 * @param[in] name 
		 * @return int 
		 */
		int getUniformLocation(const std::string &name) const;

		/**
		 * @brief Get the Shaders object
		 * 
		 * @note Reads the given files and set them 
		 * 
		 * @param[in] vertexFileName 
		 * @param[in] fragmentFileName 
		 * @param[in] geometryFileName 
		 * @return ProgramShaders 
		 */
		ProgramShaders getShaders(
			const std::string &vertexFileName,
			const std::string &fragmentFileName,
			const std::string &geometryFileName
		) const;

		/**
		 * @brief Compiles the Shader object
		 * 
		 * @note https://docs.gl/es3/glCompileShader
		 * There is a check if the shader in the Shader 
		 * object IS valid. If there is mistake it will 
		 * be printed 
		 * 
		 * @param[in] type 
		 * @param[in] source 
		 * @return uint32_t 
		 */
		uint32_t CompileShader(uint32_t type, const std::string &source) const;

		/**
		 * @brief Create a Shader object
		 * 
		 * @note Here all the shaders are compiled and 
		 * linked and validated 
		 * https://docs.gl/es3/glCreateProgram 
		 * https://docs.gl/es3/glAttachShader 
		 * https://docs.gl/es3/glLinkProgram 
		 * https://docs.gl/es3/glValidateProgram 
		 * https://docs.gl/es3/glDeleteShader 
		 * 
		 * @param[in] vertexShader 
		 * @param[in] fragmentShader 
		 * @param[in] geometryShader 
		 * @return uint32_t 
		 */
		uint32_t createShader(
			const std::string &vertexShader,
			const std::string &fragmentshader,
			const std::string &geometryshader
		) const;

	private:
		mutable std::unordered_map<std::string, int> mUniformLocationCache;

		std::string mVertexFilePath;
		std::string mFragmentFilePath;
		std::string mGeometryFilePath;

		std::string mPath;

		bool mUpdateFlag;

		uint32_t mID;
};
