#include "gl_shader.h"

#include <filesystem>
#include <stdexcept>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

#include "file_read.h"

namespace fs = std::filesystem;

namespace Core {
    ShaderSource::ShaderSource()
        : m_path()
        , vertexShader()
        , fragmentShader()
        , geometryShader() {}

    ShaderSource::ShaderSource(const std::string& path)
        : m_path(path)
        , vertexShader()
        , fragmentShader()
        , geometryShader() {
        const std::string vertexShaderName = (fs::path(m_path) / "vertexShader.shader").string();
        const std::string fragmentShaderName = (fs::path(m_path) / "fragmentShader.shader").string();
        std::string geometryShaderName = (fs::path(m_path) / "geometryShader.shader").string();

        if (!validatePaths(
            vertexShaderName,
            fragmentShaderName,
            geometryShaderName
        )) {
            throw std::runtime_error("Shader source validation failed");
        }

        vertexShader = fileToString(vertexShaderName);
        fragmentShader = fileToString(fragmentShaderName);

        if (!geometryShaderName.empty()) {
            geometryShader = fileToString(geometryShaderName);
        }
    }

    bool ShaderSource::validatePaths(
        const std::string& vertexShaderName,
        const std::string& fragmentShaderName,
        std::string& geometryShaderName
    ) {
        if (!fs::exists(vertexShaderName)) {
            LOG_FATAL("Shader '%s' missing vertex shader at '%s'", m_path.c_str(), vertexShaderName.c_str());
            return false;
        }
        if (!fs::exists(fragmentShaderName)) {
            LOG_FATAL("Shader '%s' missing fragment shader at '%s'", m_path.c_str(), fragmentShaderName.c_str());
            return false;
        }
        if (!fs::exists(geometryShaderName)) {
            LOG_INFO("Shader '%s' running without geometry shader", m_path.c_str());
            geometryShaderName.clear();
        }
        return true;
    }

    Shader::Shader(const std::string& path)
        : GLObject(GL_NONE, GL_PROGRAM, 0)
        , m_name()
        , m_path(path)
        , m_source()
        , m_uniformLocationCache() {
        if (!fs::exists(m_path)) {
            LOG_ERROR("Shader path '%s' does not exist", m_path.c_str());
        }

        fs::path p = fs::path(m_path);
        m_name = p.filename().string();

        m_source = ShaderSource(m_path);

        createShader();
    }

    Shader::~Shader() {
        if (m_id == 0) {
            LOG_FATAL("Attempting to delete invalid shader program [ID:%u]", m_id);
            VKM_ASSERT(false);
        }

        VKM_GL_CHECK(glDeleteProgram(m_id));
    }

    void Shader::bind(GLenum target) const {
        (void)target;
        VKM_GL_CHECK(glUseProgram(m_id));
    }

    void Shader::unbind(GLenum target) const {
        (void)target;
        VKM_GL_CHECK(glUseProgram(0));
    }

    void Shader::reCompleShader() {
        m_source = ShaderSource(m_path);
        m_uniformLocationCache.clear();

        if (m_id != 0) {
            VKM_GL_CHECK(glDeleteProgram(m_id));
            m_id = 0;
        }

        createShader();
    }

    const std::string& Shader::getName() const {
        return m_name;
    }

    const std::string& Shader::getPath() const {
        return m_path;
    }

    void Shader::createShader() {
        m_id = VKM_GL_CHECK(glCreateProgram());
        VKM_ASSERT(m_id != 0);

        const uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, m_source.vertexShader);
        const uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, m_source.fragmentShader);
        uint32_t geometryShader = 0;

        if (!m_source.geometryShader.empty()) {
            geometryShader = compileShader(GL_GEOMETRY_SHADER, m_source.geometryShader);
        }

        VKM_GL_CHECK(glAttachShader(m_id, vertexShader));
        VKM_GL_CHECK(glAttachShader(m_id, fragmentShader));

        if (geometryShader != 0) {
            VKM_GL_CHECK(glAttachShader(m_id, geometryShader));
        }

        VKM_GL_CHECK(glLinkProgram(m_id));
        VKM_GL_CHECK(glValidateProgram(m_id));

        int32_t linkStatus = 0;
        VKM_GL_CHECK(glGetProgramiv(m_id, GL_LINK_STATUS, &linkStatus));
        if (linkStatus != GL_TRUE) {
            int32_t infoLogLength = 0;
            VKM_GL_CHECK(glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength));
            std::string errorMessage(static_cast<size_t>(infoLogLength), '\0');
            VKM_GL_CHECK(glGetProgramInfoLog(m_id, infoLogLength, &infoLogLength, errorMessage.data()));
            LOG_FATAL("Shader program link failed [%s]: %s", m_name.c_str(), errorMessage.c_str());
            throw std::runtime_error("Shader program link failed");
        }

        // Apply a debug label for easier GPU debugging.
        setLabel(m_name.c_str());

        VKM_GL_CHECK(glDeleteShader(vertexShader));
        VKM_GL_CHECK(glDeleteShader(fragmentShader));

        if (geometryShader != 0) {
            VKM_GL_CHECK(glDeleteShader(geometryShader));
        }

        LOG_INFO("Shader '%s' successfully created", m_path.c_str());
    }

    uint32_t Shader::compileShader(uint32_t type, const std::string& source) const {
        uint32_t id = VKM_GL_CHECK(glCreateShader(type));
        const char* src = source.c_str();

        VKM_GL_CHECK(glShaderSource(id, 1, &src, nullptr));
        VKM_GL_CHECK(glCompileShader(id));

        int32_t result = 0;
        VKM_GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

        if (result != GL_TRUE) {
            return printError(type, id);
        }

        return id;
    }

    uint32_t Shader::printError(uint32_t type, uint32_t id) const {
        std::string shaderTypeName = "UNKNOWN";

        if (type == GL_VERTEX_SHADER) {
            shaderTypeName = "VERTEX";
        } else if (type == GL_FRAGMENT_SHADER) {
            shaderTypeName = "FRAGMENT";
        } else if (type == GL_GEOMETRY_SHADER) {
            shaderTypeName = "GEOMETRY";
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

    int32_t Shader::getUniformLocation(const std::string& name) const {
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

    void Shader::setUniform4f(const std::string& uniformName, float f0, float f1, float f2, float f3) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform4f(location, f0, f1, f2, f3));
    }

    void Shader::setUniform3f(const std::string& uniformName, float f0, float f1, float f2) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform3f(location, f0, f1, f2));
    }

    void Shader::setUniform2f(const std::string& uniformName, float f0, float f1) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform2f(location, f0, f1));
    }

    void Shader::setUniform1f(const std::string& uniformName, float f0) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform1f(location, f0));
    }

    void Shader::setUniform4fv(const std::string& uniformName, const glm::vec4& v) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform4fv(location, 1, &v[0]));
    }

    void Shader::setUniform3fv(const std::string& uniformName, const glm::vec3& v) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform3fv(location, 1, &v[0]));
    }

    void Shader::setUniform2fv(const std::string& uniformName, const glm::vec2& v) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform2fv(location, 1, &v[0]));
    }

    void Shader::setUniformMatrix4fv(const std::string& uniformName, const glm::mat4& matrix) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
    }

    void Shader::setUniformMatrix3fv(const std::string& uniformName, const glm::mat3& matrix) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniformMatrix3fv(location, 1, GL_FALSE, &matrix[0][0]));
    }

    void Shader::setUniform1ui(const std::string& uniformName, uint32_t ui0) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform1ui(location, ui0));
    }

    void Shader::setUniform1i(const std::string& uniformName, int32_t i0) const {
        const int32_t location = getUniformLocation(uniformName);
        VKM_GL_CHECK(glUniform1i(location, i0));
    }
};
