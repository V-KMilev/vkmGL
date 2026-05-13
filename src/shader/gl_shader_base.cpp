#include "gl_shader_base.h"

#include <filesystem>
#include <stdexcept>
#include <utility>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace fs = std::filesystem;

namespace Core {

ShaderBase::ShaderBase(const std::string& path)
    : GLObject(GL_NONE, GL_PROGRAM, 0)
    , m_name()
    , m_path(path)
    , m_uniformLocationCache() {
    if (!fs::exists(m_path)) {
        LOG_ERROR("Shader path '%s' does not exist", m_path.c_str());
    }

    fs::path p = fs::path(m_path);
    m_name = p.filename().string();
}

ShaderBase::~ShaderBase() {
    release();
}

ShaderBase& ShaderBase::operator=(ShaderBase&& other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
        m_name                 = std::move(other.m_name);
        m_path                 = std::move(other.m_path);
        m_uniformLocationCache = std::move(other.m_uniformLocationCache);
    }
    return *this;
}

void ShaderBase::release() noexcept {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteProgram(m_id));
    m_id = 0;
}

void ShaderBase::bind(GLenum target) const {
    VKM_GL_CHECK(glUseProgram(m_id));
}

void ShaderBase::unbind(GLenum target) const {
    VKM_GL_CHECK(glUseProgram(0));
}

void ShaderBase::recompile() {
    reloadSource();
    m_uniformLocationCache.clear();
    release();
    createProgram();
}

const std::string& ShaderBase::getName() const {
    return m_name;
}

const std::string& ShaderBase::getPath() const {
    return m_path;
}

uint32_t ShaderBase::compileShader(uint32_t type, const std::string& source) const {
    uint32_t id = VKM_GL_CHECK(glCreateShader(type));
    const char* src = source.c_str();

    VKM_GL_CHECK(glShaderSource(id, 1, &src, nullptr));
    VKM_GL_CHECK(glCompileShader(id));

    int32_t result = 0;
    VKM_GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

    if (result != GL_TRUE) {
        printCompilationError(type, id);
    }

    return id;
}

void ShaderBase::linkProgram(uint32_t programId) const {
    VKM_GL_CHECK(glLinkProgram(programId));
    VKM_GL_CHECK(glValidateProgram(programId));

    int32_t linkStatus = 0;
    VKM_GL_CHECK(glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus));
    if (linkStatus != GL_TRUE) {
        int32_t infoLogLength = 0;
        VKM_GL_CHECK(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength));
        std::string errorMessage(static_cast<size_t>(infoLogLength), '\0');
        VKM_GL_CHECK(glGetProgramInfoLog(programId, infoLogLength, &infoLogLength, errorMessage.data()));
        LOG_FATAL("Shader program link failed [%s]: %s", m_name.c_str(), errorMessage.c_str());
        throw std::runtime_error("Shader program link failed");
    }
}

void ShaderBase::printCompilationError(uint32_t type, uint32_t id) const {
    std::string shaderTypeName = "UNKNOWN";

    if (type == GL_VERTEX_SHADER) {
        shaderTypeName = "VERTEX";
    } else if (type == GL_FRAGMENT_SHADER) {
        shaderTypeName = "FRAGMENT";
    } else if (type == GL_GEOMETRY_SHADER) {
        shaderTypeName = "GEOMETRY";
    } else if (type == GL_COMPUTE_SHADER) {
        shaderTypeName = "COMPUTE";
    }

    LOG_ERROR("Shader '%s' failed to compile %s shader [ID:%u]", m_path.c_str(), shaderTypeName.c_str(), id);

    int32_t infoLogLength = 0;
    VKM_GL_CHECK(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength));
    std::string errorMessage(static_cast<size_t>(infoLogLength), '\0');
    VKM_GL_CHECK(glGetShaderInfoLog(id, infoLogLength, &infoLogLength, errorMessage.data()));

    LOG_ERROR("Shader compilation log: %s", errorMessage.c_str());

    VKM_GL_CHECK(glDeleteShader(id));

    throw std::runtime_error("Shader compilation failed");
}

int32_t ShaderBase::getUniformLocation(const std::string& name) const {
    const auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }

    int32_t location = glGetUniformLocation(m_id, name.c_str());

    if (location == -1) {
        LOG_WARNING("Shader '%s' uniform '%s' does not exist", m_name.c_str(), name.c_str());
    }

    m_uniformLocationCache[name] = location;
    return location;
}

void ShaderBase::setUniform4f(const std::string& uniformName, float f0, float f1, float f2, float f3) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform4f(location, f0, f1, f2, f3));
    }
}

void ShaderBase::setUniform3f(const std::string& uniformName, float f0, float f1, float f2) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform3f(location, f0, f1, f2));
    }
}

void ShaderBase::setUniform2f(const std::string& uniformName, float f0, float f1) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform2f(location, f0, f1));
    }
}

void ShaderBase::setUniform1f(const std::string& uniformName, float f0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform1f(location, f0));
    }
}

void ShaderBase::setUniform4fv(const std::string& uniformName, const glm::vec4& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform4fv(location, 1, &v[0]));
    }
}

void ShaderBase::setUniform3fv(const std::string& uniformName, const glm::vec3& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform3fv(location, 1, &v[0]));
    }
}

void ShaderBase::setUniform2fv(const std::string& uniformName, const glm::vec2& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform2fv(location, 1, &v[0]));
    }
}

void ShaderBase::setUniformMatrix4fv(const std::string& uniformName, const glm::mat4& matrix) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
    }
}

void ShaderBase::setUniformMatrix3fv(const std::string& uniformName, const glm::mat3& matrix) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniformMatrix3fv(location, 1, GL_FALSE, &matrix[0][0]));
    }
}

void ShaderBase::setUniform1ui(const std::string& uniformName, uint32_t ui0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform1ui(location, ui0));
    }
}

void ShaderBase::setUniform1i(const std::string& uniformName, int32_t i0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform1i(location, i0));
    }
}

};
