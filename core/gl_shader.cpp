#include "gl_shader.h"

#include "error/gl_error_handle.h"

#include "error/error_handle.h"
#include "files/file_read.h"

ProgramShaders::ProgramShaders(
	const std::string &vertexShader,
	const std::string &fragmentShader,
	const std::string &geometryShader
) : vertexShader(vertexShader),
	fragmentShader(fragmentShader),
	geometryShader(geometryShader) {}

Shader::Shader(
	const std::string &vertexFileName,
	const std::string &fragmentFileName,
	const std::string &geometryFileName
)
 : mVertexFilePath(vertexFileName)
 , mFragmentFilePath(fragmentFileName)
 , mGeometryFilePath(geometryFileName)
 , mUpdateFlag(false)
 , mID(0) {
	mPath = vertexFileName.substr(0, vertexFileName.find_last_of('\\'));

	ProgramShaders source = getShaders(vertexFileName, fragmentFileName, geometryFileName);
	mID = createShader(source.vertexShader, source.fragmentShader, source.geometryShader);

	M_ASSERT(mID != 0);
}

Shader::~Shader() {
	MY_GL_CHECK(glDeleteProgram(mID));
}

void Shader::bind() const {
	MY_GL_CHECK(glUseProgram(mID));
}

void Shader::unbind() const {
	MY_GL_CHECK(glUseProgram(0));
}



void Shader::setUniform4f(
	const std::string &uniformName,
	float f0,
	float f1,
	float f2,
	float f3
) const {

	int location = getUniformLocation(uniformName);

	MY_GL_CHECK(glUniform4f(location, f0, f1, f2, f3));
}

void Shader::setUniform3f(
	const std::string &uniformName,
	float f0,
	float f1,
	float f2
) const {

	int location = getUniformLocation(uniformName);

	MY_GL_CHECK(glUniform3f(location, f0, f1, f2));
}

void Shader::setUniform1f(const std::string &uniformName, float f0) const {

	int location = getUniformLocation(uniformName);

	MY_GL_CHECK(glUniform1f(location, f0));
}

void Shader::setUniform3fv(const std::string &uniformName, const glm::vec3& v0) const {

	int location = getUniformLocation(uniformName);

	MY_GL_CHECK(glUniform3fv(location, 1, &v0[0]));
}

void Shader::setUniformMatrix4fv(const std::string &uniformName, const glm::mat4 &matrix) const {

	int location = getUniformLocation(uniformName);

	MY_GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
}

void Shader::setUniform1i(const std::string &uniformName, int i0) const {

	int location = getUniformLocation(uniformName);

	MY_GL_CHECK(glUniform1i(location, i0));
}

void Shader::setUniform1ui(const std::string &uniformName, uint32_t ui0) const {

	int location = getUniformLocation(uniformName);

	MY_GL_CHECK(glUniform1ui(location, ui0));
}

int Shader::getUniformLocation(const std::string &name) const {
	// If location already exist we just return it
	if(mUniformLocationCache.find(name) != mUniformLocationCache.end()) {
		return mUniformLocationCache[name];
	}

	int location = glGetUniformLocation(mID, &name[0]);

	// -1 means we dont have such uniform
	// We want to store the uiforms that are not existing so the program can continue
	if(location == -1) {
		// TODO: At some point fix this not to show only the fragment shader path
		std::cerr << "UNIFORM WARNING: Uniform at [" << mPath << "]: " << name << "' doesn't exist!\n";
	}

	mUniformLocationCache[name] = location;
	return location;
}



ProgramShaders Shader::getShaders(
	const std::string &vertexFileName,
	const std::string &fragmentFileName,
	const std::string &geometryFileName
) const {
	const std::string vertexShader   = fileToString(vertexFileName);
	const std::string fragmentShader = fileToString(fragmentFileName);
	const std::string geometryShader = fileToString(geometryFileName);

	ProgramShaders shaders(vertexShader, fragmentShader, geometryShader);

	return shaders;
}

uint32_t Shader::CompileShader(uint32_t type, const std::string &source) const {

	uint32_t id = glCreateShader(type);
	const char* src = &source[0];    // the same as source.c_str();

	MY_GL_CHECK(glShaderSource(id, 1, &src, nullptr));
	MY_GL_CHECK(glCompileShader(id));

	// ERROR handling
	int result = 0;
	MY_GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &result));    // wants int vector

	// GL_FALSE = 0
	if(result != GL_TRUE) {

		int length;
		MY_GL_CHECK(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));    // set length size

		char* message = (char*) alloca(length * sizeof(char));
		MY_GL_CHECK(glGetShaderInfoLog(id, length, &length, message));

		std::cerr << "SHADER FAILED: Shader [" << mPath << "] failed!\n";

		if(type == GL_VERTEX_SHADER)   { std::cerr << "SHADER FAILED: Failed to complie VERTEX shader:\n"; }
		if(type == GL_FRAGMENT_SHADER) { std::cerr << "SHADER FAILED: Failed to complie FRAGMENT shader:\n"; }
		if(type == GL_GEOMETRY_SHADER) { std::cerr << "SHADER FAILED: Failed to complie GEOMETRY shader:\n"; }

		std::cerr << message << "\n";

		MY_GL_CHECK(glDeleteShader(id));

		return 0;
	}

	return id;
}

uint32_t Shader::createShader(
	const std::string &vertexShader,
	const std::string &fragmentShader,
	const std::string &geometryShader
) const {
	// Create new progam
	uint32_t program = glCreateProgram();

	uint32_t vertexShad   = 0;
	uint32_t fragmentShad = 0;
	uint32_t geometryShad = 0;

	vertexShad   = CompileShader(GL_VERTEX_SHADER, vertexShader);                                     // compile vertex shader
	fragmentShad = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);                                 // compile fragment shader
	if(!geometryShader.empty()) { geometryShad = CompileShader(GL_GEOMETRY_SHADER, geometryShader); } // compile geometry shader

	MY_GL_CHECK(glAttachShader(program, vertexShad));                                   // add vertex shader
	MY_GL_CHECK(glAttachShader(program, fragmentShad));                                 // add fragment shader
	if(!geometryShader.empty()) { MY_GL_CHECK(glAttachShader(program, geometryShad)); } // add geometry shader

	MY_GL_CHECK(glLinkProgram(program));     // link program
	MY_GL_CHECK(glValidateProgram(program)); // validate program

	MY_GL_CHECK(glDeleteShader(vertexShad));                                   // delete vertex shader
	MY_GL_CHECK(glDeleteShader(fragmentShad));                                 // delete fragment shader
	if(!geometryShader.empty()) { MY_GL_CHECK(glDeleteShader(geometryShad)); } // delete geometry shader

	std::cerr << "SHADER CREATE: Shader [" << program << "][" << mPath << "] successfully created!\n";

	return program;
}

void Shader::reCompleShader() {
	ProgramShaders source = getShaders(mVertexFilePath, mFragmentFilePath, mGeometryFilePath);

	// Clear the old cache so we only have the updated
	mUniformLocationCache.clear();

	// Create new progam
	uint32_t program = glCreateProgram();

	uint32_t vertexShad   = 0;
	uint32_t fragmentShad = 0;
	uint32_t geometryShad = 0;

	vertexShad   = CompileShader(GL_VERTEX_SHADER, source.vertexShader);                                            // compile vertex shader
	fragmentShad = CompileShader(GL_FRAGMENT_SHADER, source.fragmentShader);                                        // compile fragment shader
	if(!source.geometryShader.empty()) { geometryShad = CompileShader(GL_GEOMETRY_SHADER, source.geometryShader); } // compile geometry shader

	MY_GL_CHECK(glAttachShader(program, vertexShad));                                          // add vertex shader
	MY_GL_CHECK(glAttachShader(program, fragmentShad));                                        // add fragment shader
	if(!source.geometryShader.empty()) { MY_GL_CHECK(glAttachShader(program, geometryShad)); } // add geometry shader

	MY_GL_CHECK(glLinkProgram(program));     // link program
	MY_GL_CHECK(glValidateProgram(program)); // validate program

	MY_GL_CHECK(glDeleteShader(vertexShad));                                          // delete vertex shader
	MY_GL_CHECK(glDeleteShader(fragmentShad));                                        // delete fragment shader
	if(!source.geometryShader.empty()) { MY_GL_CHECK(glDeleteShader(geometryShad)); } // delete geometry shader

	// Set the new program as current
	mID = program;

	std::cerr << "SHADER RECOMPLE: Shader [" << program << "][" << mPath << "] successfully recompled!\n";
}

void Shader::setUpdateFlag(bool value) { mUpdateFlag = value; }

const std::string& Shader::getPath() const { return mPath; }
bool Shader::getUpdateFlag()         const { return mUpdateFlag; }
