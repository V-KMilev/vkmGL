#include "gl_shader.h"

#include <stdio.h>

#include "gl_error_handle.h"
#include "error_handle.h"

#include "file_read.h"

namespace Core {
	ShaderSource::ShaderSource() :
		vertexShader(""),
		fragmentShader(""),
		geometryShader("") {}

	ShaderSource::ShaderSource(
		const std::string& vertexShader,
		const std::string& fragmentShader,
		const std::string& geometryShader
	) : vertexShader(vertexShader),
		fragmentShader(fragmentShader),
		geometryShader(geometryShader) {}

	Shader::Shader(const std::string& path) :
		_mID(0),
		_mPath(path),
		_mSource(),
		_mUniformLocationCache({})
	{
		_mSource = ShaderSource(
			path + "fragmentShader.shader",
			path + "vertexShader.shader",
			path + "geometryShader.shader"
		);

		_mID = createShader();

		M_ASSERT(_mID != 0);
	}

	Shader::~Shader() {
		MY_GL_CHECK(glDeleteProgram(_mID));
	}

	void Shader::bind() const {
		MY_GL_CHECK(glUseProgram(_mID));
	}

	void Shader::unbind() const {
		MY_GL_CHECK(glUseProgram(0));
	}

	void Shader::reCompleShader() {
		// Clear the old cache
		_mUniformLocationCache.clear();

		MY_GL_CHECK(glDeleteProgram(_mID));

		createShader();
	}

	unsigned int Shader::getID() const {
		return _mID;
	}

	const std::string& Shader::getPath() const {
		return _mPath;
	}

	unsigned int Shader::createShader() const {
		// Create a new program
		unsigned int program = glCreateProgram();

		unsigned int vertexShader   = 0;
		unsigned int fragmentShader = 0;
		unsigned int geometryShader = 0;

		// Compile the shaders
		vertexShader = CompileShader(GL_VERTEX_SHADER, _mSource.vertexShader);
		fragmentShader = CompileShader(GL_FRAGMENT_SHADER, _mSource.fragmentShader);

		if (!_mSource.geometryShader.empty()) {
			geometryShader = CompileShader(GL_GEOMETRY_SHADER, _mSource.geometryShader);
		}

		// Attach the shaders
		MY_GL_CHECK(glAttachShader(program, vertexShader));
		MY_GL_CHECK(glAttachShader(program, fragmentShader));

		if (!_mSource.geometryShader.empty()) {
			MY_GL_CHECK(glAttachShader(program, geometryShader));
		}

		MY_GL_CHECK(glLinkProgram(program));
		MY_GL_CHECK(glValidateProgram(program));

		// Delete the shaders
		MY_GL_CHECK(glDeleteShader(vertexShader));
		MY_GL_CHECK(glDeleteShader(fragmentShader));

		if (!_mSource.geometryShader.empty()) {
			MY_GL_CHECK(glDeleteShader(geometryShader));
		}

		printf("[INFO:CORE] Shader: '%s' [ID:%u] successfully created!\n", _mPath.c_str(), program);

		return program;
	}

	unsigned int Shader::CompileShader(unsigned int type, const std::string& source) const {
		unsigned int id = glCreateShader(type);
		// The same as source.c_str();
		const char* src = &source[0];

		MY_GL_CHECK(glShaderSource(id, 1, &src, nullptr));
		MY_GL_CHECK(glCompileShader(id));

		int result = 0;
		MY_GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

		if(result != GL_TRUE) {
			return printError(type, id);
		}

		return id;
	}

	int Shader::printError(unsigned int type, unsigned int id) const {
		std::string shaderTypeName;

		if      (type == GL_VERTEX_SHADER)   { shaderTypeName = "VERTEX"; }
		else if (type == GL_FRAGMENT_SHADER) { shaderTypeName = "FRAGMENT"; }
		else if (type == GL_GEOMETRY_SHADER) { shaderTypeName = "GEOMETRY"; }

		printf("[ERROR:CORE] Shader: %s, Failed to compile %s shader!\n", _mPath.c_str(), shaderTypeName.c_str());

		// Get error message
		int infoLogLength;
		MY_GL_CHECK(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength));

		char* errorMessage = (char*)alloca(infoLogLength * sizeof(char));
		MY_GL_CHECK(glGetShaderInfoLog(id, infoLogLength, &infoLogLength, errorMessage));

		printf("[ERROR:CORE] Error message: %s\n", errorMessage);

		MY_GL_CHECK(glDeleteShader(id));

		return 0;
	}

	int Shader::getUniformLocation(const std::string& name) const {
		// If location already exist we just return it
		if(_mUniformLocationCache.find(name) != _mUniformLocationCache.end()) {
			return _mUniformLocationCache[name];
		}

		int location = glGetUniformLocation(_mID, &name[0]);

		// -1 means we dont have such uniform
		// We want to store the uiforms that are not existing so the program can continue
		if(location == -1) {
			printf("[WARN:CORE] Shader: %s, Uniform: '%s' does not exist!\n", _mPath.c_str(), name.c_str());
		}

		_mUniformLocationCache[name] = location;
		return location;
	}

	void Shader::setUniform4f(const std::string& uniformName, float f0, float f1, float f2, float f3) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform4f(location, f0, f1, f2, f3));
	}

	void Shader::setUniform3f(const std::string& uniformName, float f0, float f1, float f2) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform3f(location, f0, f1, f2));
	}

	void Shader::setUniform2f(const std::string& uniformName, float f0, float f1) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform2f(location, f0, f1));
	}

	void Shader::setUniform1f(const std::string& uniformName, float f0) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform1f(location, f0));
	}

	void Shader::setUniform4fv(const std::string& uniformName, const glm::vec4& v) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform4fv(location, 1, &v[0]));
	}

	void Shader::setUniform3fv(const std::string& uniformName, const glm::vec3& v) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform3fv(location, 1, &v[0]));
	}

	void Shader::setUniform2fv(const std::string& uniformName, const glm::vec2& v) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform2fv(location, 1, &v[0]));
	}

	void Shader::setUniformMatrix4fv(const std::string& uniformName, const glm::mat4& matrix) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
	}

	void Shader::setUniformMatrix3fv(const std::string& uniformName, const glm::mat3& matrix) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniformMatrix3fv(location, 1, GL_FALSE, &matrix[0][0]));
	}

	void Shader::setUniform1ui(const std::string& uniformName, unsigned int ui0) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform1ui(location, ui0));
	}

	void Shader::setUniform1i(const std::string& uniformName, int i0) const {
		int location = getUniformLocation(uniformName);
		MY_GL_CHECK(glUniform1i(location, i0));
	}
};
