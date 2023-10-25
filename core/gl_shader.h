#pragma once

#include <unordered_map>
#include <string>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/rotate_vector.hpp"

namespace Core {
	struct ShaderSource {
		ShaderSource();
		~ShaderSource() = default;

		ShaderSource (const std::string& path);

		void validShaderPaths(
			const std::string& vertexShaderName,
			const std::string& fragmentShaderName,
			std::string& geometryShaderName
		);

		std::string _mPath;

		std::string vertexShader;
		std::string fragmentShader;
		std::string geometryShader;
	};

	class Shader {
		public:
			Shader() = delete;
			~Shader();

			Shader(const std::string& path);

			Shader(const Shader& shader) = delete;
			Shader& operator = (const Shader& shader) = delete;

			Shader(Shader && shader) = delete;
			Shader& operator = (Shader && shader) = delete;

			void bind() const;
			void unbind() const;

			void reCompleShader();

			unsigned int getID() const;

			const std::string& getPath() const;

			void setUniform4f(const std::string& uniformName, float f0, float f1, float f2, float f3) const;
			void setUniform3f(const std::string& uniformName, float f0, float f1, float f2) const;
			void setUniform2f(const std::string& uniformName, float f0, float f1) const;
			void setUniform1f(const std::string& uniformName, float f0) const;

			void setUniform4fv(const std::string& uniformName, const glm::vec4& v) const;
			void setUniform3fv(const std::string& uniformName, const glm::vec3& v) const;
			void setUniform2fv(const std::string& uniformName, const glm::vec2& v) const;

			void setUniformMatrix4fv(const std::string& uniformName, const glm::mat4& matrix) const;
			void setUniformMatrix3fv(const std::string& uniformName, const glm::mat3& matrix) const;

			void setUniform1ui(const std::string& uniformName, unsigned int ui0) const;

			void setUniform1i(const std::string& uniformName, int i0) const;

		private:
			void createShader();

			unsigned int CompileShader(unsigned int type, const std::string& source) const;

			int printError(unsigned int type, unsigned int id) const;

			int getUniformLocation(const std::string& name) const;

		private:
			unsigned int _mID;

			std::string _mPath;
			ShaderSource _mSource;

			mutable std::unordered_map<std::string, int> _mUniformLocationCache;
	};
};
