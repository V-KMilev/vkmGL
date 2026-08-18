#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "SHADER"

#include "gl_shader_base.h"

#include "gl_shader_reload.h"

#include <filesystem>
#include <stdexcept>
#include <utility>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace fs = std::filesystem;

namespace Vkm::GL {

ShaderBase::ShaderBase(const std::string& path)
    : GLObject(GL_NONE, GL_PROGRAM, 0)
    , m_name()
    , m_path(path)
    , m_uniformNames()
    , m_uniformLocationCache() {
    if (!fs::exists(m_path)) {
        LOG_ERROR("Shader path '%s' does not exist", m_path.c_str());
    }

    fs::path p = fs::path(m_path);
    m_name = p.filename().string();

    registerShader(this);
}

ShaderBase::~ShaderBase() {
    unregisterShader(this);
    release();
}

ShaderBase& ShaderBase::operator=(ShaderBase && other) noexcept {
    if (this != &other) {
        release();
        GLObject::operator=(std::move(other));
        m_name                 = std::move(other.m_name);
        m_path                 = std::move(other.m_path);
        // GLObject::operator= above moved other's program id into this, so the
        // cached locations still match. Move the backing names first: deque
        // move preserves element addresses, keeping the view keys valid.
        m_uniformNames         = std::move(other.m_uniformNames);
        m_uniformLocationCache = std::move(other.m_uniformLocationCache);
    }
    return *this;
}

void ShaderBase::release() noexcept {
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteProgram(m_id));
    m_id = 0;
}

void ShaderBase::bind() const {
    VKM_GL_CHECK(glUseProgram(m_id));
}

void ShaderBase::unbind() const {
    VKM_GL_CHECK(glUseProgram(0));
}

bool ShaderBase::tryRecompile() {
    const uint32_t previous = m_id;

    try {
        reloadSource();
        // createProgram() assigns a fresh name; clearing this first means a
        // throw cannot leave us pointing at the old program's id.
        m_id = 0;
        createProgram();
    } catch (const std::exception& e) {
        // Drop whatever half-built program we made, restore the one that works.
        if (m_id != 0 && m_id != previous) {
            VKM_GL_CHECK(glDeleteProgram(m_id));
        }
        m_id = previous;
        LOG_ERROR_C("SHADER", "Reload of '%s' failed, keeping the previous program: %s",
                    m_name.c_str(), e.what());
        return false;
    }

    // Linked: the old program is now unreferenced, and the caches describe it.
    if (previous != 0) {
        VKM_GL_CHECK(glDeleteProgram(previous));
    }
    m_uniformLocationCache.clear();
    m_uniformNames.clear();
    return true;
}

void ShaderBase::recompile() {
    reloadSource();
    // The relinked program reassigns uniform locations, so drop the cache and
    // its backing names; both repopulate lazily on the next lookup.
    m_uniformLocationCache.clear();
    m_uniformNames.clear();
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
    uint32_t id = 0;
    VKM_GL_CHECK(id = glCreateShader(type));
    const char* src = source.c_str();

    VKM_GL_CHECK(glShaderSource(id, 1, &src, nullptr));
    VKM_GL_CHECK(glCompileShader(id));

    int32_t result = 0;
    VKM_GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

    if (result != GL_TRUE) {
        throwCompilationError(type, id);
    }

    return id;
}

void ShaderBase::linkProgram(uint32_t programId) const {
    VKM_GL_CHECK(glLinkProgram(programId));

    int32_t linkStatus = 0;
    VKM_GL_CHECK(glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus));
    if (linkStatus != GL_TRUE) {
        int32_t infoLogLength = 0;
        VKM_GL_CHECK(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength));
        std::string errorMessage(static_cast<size_t>(infoLogLength), '\0');
        VKM_GL_CHECK(glGetProgramInfoLog(programId, infoLogLength, &infoLogLength, errorMessage.data()));
        // Not fatal: tryRecompile() catches this so a hot-reload edit that does
        // not link keeps the working program. Only an uncaught throw - startup -
        // ends the process, and that is the caller's policy, not ours.
        LOG_ERROR("Shader program link failed [%s]: %s", m_name.c_str(), errorMessage.c_str());
        throw std::runtime_error("Shader program link failed");
    }
}

void ShaderBase::throwCompilationError(uint32_t type, uint32_t id) const {
    const char* stageName = "UNKNOWN";
    switch (type) {
        case GL_VERTEX_SHADER:   stageName = "VERTEX";   break;
        case GL_FRAGMENT_SHADER: stageName = "FRAGMENT"; break;
        case GL_GEOMETRY_SHADER: stageName = "GEOMETRY"; break;
        case GL_COMPUTE_SHADER:  stageName = "COMPUTE";  break;
        default: break;
    }

    LOG_ERROR("Shader '%s' failed to compile %s shader [ID:%u]", m_path.c_str(), stageName, id);

    int32_t infoLogLength = 0;
    VKM_GL_CHECK(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength));
    std::string errorMessage(static_cast<size_t>(infoLogLength), '\0');
    VKM_GL_CHECK(glGetShaderInfoLog(id, infoLogLength, &infoLogLength, errorMessage.data()));

    LOG_ERROR("Shader compilation log: %s", errorMessage.c_str());

    VKM_GL_CHECK(glDeleteShader(id));

    throw std::runtime_error("Shader compilation failed");
}

int32_t ShaderBase::resolveUniform(const char* name, bool warnIfMissing) const {
    const auto it = m_uniformLocationCache.find(std::string_view(name));
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }

    const int32_t location = glGetUniformLocation(m_id, name);
    if (location == -1 && warnIfMissing) {
        LOG_WARNING("Shader '%s' uniform '%s' does not exist", m_name.c_str(), name);
    }

    // Own the name so the cache's view key stays valid for the shader's
    // lifetime; the deque leaves existing entries' addresses untouched.
    const std::string& owned = m_uniformNames.emplace_back(name);
    m_uniformLocationCache.emplace(std::string_view(owned), location);
    return location;
}

bool ShaderBase::hasUniform(const char* name) const {
    return resolveUniform(name, /*warnIfMissing=*/false) != -1;
}

int32_t ShaderBase::getUniformLocation(const char* name) const {
    return resolveUniform(name, /*warnIfMissing=*/true);
}

void ShaderBase::setUniform4f(const char* uniformName, float f0, float f1, float f2, float f3) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform4f(location, f0, f1, f2, f3));
    }
}

void ShaderBase::setUniform3f(const char* uniformName, float f0, float f1, float f2) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform3f(location, f0, f1, f2));
    }
}

void ShaderBase::setUniform2f(const char* uniformName, float f0, float f1) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform2f(location, f0, f1));
    }
}

void ShaderBase::setUniform1f(const char* uniformName, float f0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform1f(location, f0));
    }
}

void ShaderBase::setUniform4fv(const char* uniformName, const glm::vec4& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform4fv(location, 1, &v[0]));
    }
}

void ShaderBase::setUniform3fv(const char* uniformName, const glm::vec3& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform3fv(location, 1, &v[0]));
    }
}

void ShaderBase::setUniform2i(const char* uniformName, int32_t i0, int32_t i1) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform2i(location, i0, i1));
    }
}

void ShaderBase::setUniform2fv(const char* uniformName, const glm::vec2& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform2fv(location, 1, &v[0]));
    }
}

void ShaderBase::setUniformMatrix4fv(const char* uniformName, const glm::mat4& matrix) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
    }
}

void ShaderBase::setUniformMatrix3fv(const char* uniformName, const glm::mat3& matrix) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniformMatrix3fv(location, 1, GL_FALSE, &matrix[0][0]));
    }
}

void ShaderBase::setUniform1ui(const char* uniformName, uint32_t ui0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform1ui(location, ui0));
    }
}

void ShaderBase::setUniform1i(const char* uniformName, int32_t i0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform1i(location, i0));
    }
}

void ShaderBase::setUniform3iv(const char* uniformName, const glm::ivec3& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glUniform3iv(location, 1, &v[0]));
    }
}

} // namespace Vkm::GL
