#include "gl_shader.h"

#include <stdio.h>
#include <filesystem>

#include "gl_error_handle.h"
#include "error_handle.h"

#include "file_read.h"

namespace fs = std::filesystem;

namespace Core {
	ShaderSource::ShaderSource() :
		_mPath(""),
		vertexShader(""),
		fragmentShader(""),
		geometryShader("") {}

	ShaderSource::ShaderSource(const std::string& path) :
		_mPath(path),
		vertexShader(""),
		fragmentShader(""),
		geometryShader("") {

		std::string vertexShaderName   = _mPath + "vertexShader.shader";
		std::string fragmentShaderName = _mPath + "fragmentShader.shader";
		std::string geometryShaderName = _mPath + "geometryShader.shader";

		validShaderPaths(
			vertexShaderName,
			fragmentShaderName,
			geometryShaderName
		);

		vertexShader   = fileToString(vertexShaderName);
		fragmentShader = fileToString(fragmentShaderName);
		geometryShader = fileToString(geometryShaderName);
	}

	void ShaderSource::validShaderPaths(
		const std::string& vertexShaderName,
		const std::string& fragmentShaderName,
		std::string& geometryShaderName
	) {
		if(!fs::exists(vertexShaderName)) {
			printf("[ERROR:CORE] Shader: '%s' does not contain vertex shader!\n", _mPath.c_str());
		}
		if(!fs::exists(fragmentShaderName)) {
			printf("[ERROR:CORE] Shader: '%s' does not contain fragment shader!\n", _mPath.c_str());
		}
		if(!fs::exists(geometryShaderName)) {
			printf("[INFO:CORE] Shader: '%s' is runnig without geometry shader!\n", _mPath.c_str());
			geometryShaderName = "";
		}
	}

	Shader::Shader(const std::string& path) :
		_mID(0),
		_mPath(path),
		_mSource(),
		_mUniformLocationCache({})
	{
		if (!fs::exists(_mPath)) {
			printf("[ERROR:CORE] Shader path: '%s' is not existing!\n", _mPath.c_str());
		}

#ifdef _WIN32
		_mName = _mPath.substr(_mPath.find_last_of('\\') + 1);
#else
		_mName = _mPath.substr(_mPath.find_last_of('/') + 1);
#endif

#ifdef _WIN32
		_mPath = _mPath + '\\';
#else
		_mPath = _mPath + '/';
#endif

		_mSource = ShaderSource(_mPath);

		createShader();
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
		// Update the source files
		_mSource = ShaderSource(_mPath);

		// Clear the old cache
		_mUniformLocationCache.clear();

		MY_GL_CHECK(glDeleteProgram(_mID));

		createShader();
	}

	unsigned int Shader::getID() const {
		return _mID;
	}

	const std::string& Shader::getName() const {
		return _mName;
	}

	const std::string& Shader::getPath() const {
		return _mPath;
	}

	void Shader::createShader() {
		// Create a new program
		_mID = MY_GL_CHECK(glCreateProgram());

		// Ensure the shader ID is valid
		M_ASSERT(_mID != 0);

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
		MY_GL_CHECK(glAttachShader(_mID, vertexShader));
		MY_GL_CHECK(glAttachShader(_mID, fragmentShader));

		if (!_mSource.geometryShader.empty()) {
			MY_GL_CHECK(glAttachShader(_mID, geometryShader));
		}

		MY_GL_CHECK(glLinkProgram(_mID));
		MY_GL_CHECK(glValidateProgram(_mID));

		// Delete the shaders
		MY_GL_CHECK(glDeleteShader(vertexShader));
		MY_GL_CHECK(glDeleteShader(fragmentShader));

		if (!_mSource.geometryShader.empty()) {
			MY_GL_CHECK(glDeleteShader(geometryShader));
		}

		printf("[INFO:CORE] Shader: '%s' successfully created!\n", _mPath.c_str());
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

		printf("[ERROR:CORE] Shader[ID:%u]: '%s' failed to compile '%s' shader[ID:%u]!\n", _mID, _mPath.c_str(), shaderTypeName.c_str(), id);

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
		// TODO: Optimize this
		const auto& it = _mUniformLocationCache.find(name);

		if (it != _mUniformLocationCache.end()) {
			return it->second;
		}

		int location = glGetUniformLocation(_mID, &name[0]);

		// -1 means we dont have such uniform
		// We want to store the uiforms that are not existing so the program can continue
		if(location == -1) {
			printf("[WARN:CORE] Shader[ID:%u]: '%s', Uniform: '%s' does not exist!\n", _mID, _mPath.c_str(), name.c_str());
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
