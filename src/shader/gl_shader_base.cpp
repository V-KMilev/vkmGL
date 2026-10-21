#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "SHADER"

#include "gl_shader_base.h"

#include "gl_shader_reload.h"

#include <algorithm>
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
        m_plain                = std::move(other.m_plain);
        m_plainListed          = std::exchange(other.m_plainListed, false);
        m_zeros                = std::move(other.m_zeros);
        m_intZeros             = std::move(other.m_intZeros);
        m_stages               = std::move(other.m_stages);
    }
    return *this;
}

void ShaderBase::release() noexcept {
    for (const uint32_t stage : m_stages) {
        VKM_GL_CHECK(glDeleteShader(stage));
    }
    m_stages.clear();
    if (m_id == 0) return;
    VKM_GL_CHECK(glDeleteProgram(m_id));
    m_id = 0;
}

namespace {

/// Whether compileInParallel() found the driver's threads.
bool g_parallel = false;

} // namespace

bool ShaderBase::compileInParallel(bool on) {
    // Any count past the driver's own maximum asks for that maximum; none, for no threads.
    const GLuint threads = on ? 0xFFFFFFFFu : 0u;
    if (GLEW_KHR_parallel_shader_compile) {
        VKM_GL_CHECK(glMaxShaderCompilerThreadsKHR(threads));
        g_parallel = on;
    } else if (GLEW_ARB_parallel_shader_compile) {
        VKM_GL_CHECK(glMaxShaderCompilerThreadsARB(threads));
        g_parallel = on;
    }
    return g_parallel;
}

bool ShaderBase::ready() const {
    if (m_stages.empty() || !g_parallel) return true;
    int32_t done = GL_FALSE;
    VKM_GL_CHECK(glGetProgramiv(m_id, GL_COMPLETION_STATUS_KHR, &done));
    return done == GL_TRUE;
}

void ShaderBase::bind() const {
    finishLink();
    VKM_GL_CHECK(glUseProgram(m_id));
}

void ShaderBase::unbind() const {
    VKM_GL_CHECK(glUseProgram(0));
}

bool ShaderBase::tryRecompile() {
    // A program still being built is waited for, so its stages are not lost to the new one's.
    try {
        finishLink();
    } catch (const std::exception&) {
        // Already logged; the reload replaces it.
    }
    const uint32_t previous = m_id;

    try {
        reloadSource();
        // createProgram() assigns a fresh name; clearing this first means a
        // throw cannot leave us pointing at the old program's id.
        m_id = 0;
        createProgram();
        finishLink();
    } catch (const std::exception& e) {
        // Drop whatever half-built program we made, restore the one that works.
        if (m_id != 0 && m_id != previous) {
            VKM_GL_CHECK(glDeleteProgram(m_id));
        }
        m_id = previous;
        LOG_ERROR_C(
            "SHADER",
            "Reload of '%s' failed, keeping the previous program: %s",
            m_name.c_str(),
            e.what()
        );
        return false;
    }

    // Linked: the old program is now unreferenced, and the caches describe it.
    if (previous != 0) {
        VKM_GL_CHECK(glDeleteProgram(previous));
    }
    m_uniformLocationCache.clear();
    m_uniformNames.clear();
    m_plainListed = false;
    return true;
}

const std::string& ShaderBase::getName() const {
    return m_name;
}

const std::string& ShaderBase::getPath() const {
    return m_path;
}

uint32_t ShaderBase::startStage(uint32_t type, const std::string& source) const {
    uint32_t id = 0;
    VKM_GL_CHECK(id = glCreateShader(type));
    const char* src = source.c_str();

    VKM_GL_CHECK(glShaderSource(id, 1, &src, nullptr));
    VKM_GL_CHECK(glCompileShader(id));
    return id;
}

void ShaderBase::startLink(std::vector<uint32_t> stages) {
    m_stages = std::move(stages);
    for (const uint32_t stage : m_stages) {
        VKM_GL_CHECK(glAttachShader(m_id, stage));
    }
    VKM_GL_CHECK(glLinkProgram(m_id));
    if (!g_parallel) finishLink();
}

void ShaderBase::finishLink() const {
    if (m_stages.empty()) return;

    // Asking a stage or the program its status waits for the driver to finish it.
    std::vector<uint32_t> stages;
    stages.swap(m_stages);
    const auto deleteStages = [&stages](uint32_t kept) {
        for (const uint32_t stage : stages) {
            if (stage != kept) VKM_GL_CHECK(glDeleteShader(stage));
        }
    };
    for (const uint32_t stage : stages) {
        int32_t compiled = GL_FALSE;
        VKM_GL_CHECK(glGetShaderiv(stage, GL_COMPILE_STATUS, &compiled));
        if (compiled == GL_TRUE) continue;
        int32_t type = 0;
        VKM_GL_CHECK(glGetShaderiv(stage, GL_SHADER_TYPE, &type));
        deleteStages(stage);
        throwCompilationError(static_cast<uint32_t>(type), stage);
    }

    int32_t linkStatus = 0;
    VKM_GL_CHECK(glGetProgramiv(m_id, GL_LINK_STATUS, &linkStatus));
    deleteStages(0);
    if (linkStatus != GL_TRUE) {
        int32_t infoLogLength = 0;
        VKM_GL_CHECK(glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength));
        std::string errorMessage(static_cast<size_t>(infoLogLength), '\0');
        VKM_GL_CHECK(glGetProgramInfoLog(m_id, infoLogLength, &infoLogLength, errorMessage.data()));
        // Not fatal: tryRecompile() catches this so a hot-reload edit that does
        // not link keeps the working program. Any other caller decides what a
        // failure does.
        LOG_ERROR("Shader program link failed [%s]: %s", m_name.c_str(), errorMessage.c_str());
        throw std::runtime_error("Shader program link failed");
    }
    LOG_TRACE("Shader '%s' linked", m_path.c_str());
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
    finishLink();
    const auto it = m_uniformLocationCache.find(std::string_view(name));
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }

    int32_t location = -1;
    VKM_GL_CHECK(location = glGetUniformLocation(m_id, name));
    if (location == -1 && warnIfMissing) {
        LOG_WARNING("Shader '%s' uniform '%s' does not exist", m_name.c_str(), name);
    }

    // Own the name so the cache's view key stays valid for the shader's
    // lifetime; the deque leaves existing entries' addresses untouched.
    const std::string& owned = m_uniformNames.emplace_back(name);
    m_uniformLocationCache.emplace(std::string_view(owned), location);
    return location;
}

namespace {

/// The values in a uniform of GL type @p type; 0 for one zeroUniforms() leaves.
uint32_t valuesIn(GLenum type) {
    switch (type) {
        case GL_FLOAT:
        case GL_INT:
        case GL_BOOL:
        case GL_UNSIGNED_INT:      return 1;
        case GL_FLOAT_VEC2:
        case GL_INT_VEC2:
        case GL_BOOL_VEC2:
        case GL_UNSIGNED_INT_VEC2: return 2;
        case GL_FLOAT_VEC3:
        case GL_INT_VEC3:
        case GL_BOOL_VEC3:
        case GL_UNSIGNED_INT_VEC3: return 3;
        case GL_FLOAT_VEC4:
        case GL_INT_VEC4:
        case GL_BOOL_VEC4:
        case GL_UNSIGNED_INT_VEC4: return 4;
        case GL_FLOAT_MAT3:        return 9;
        case GL_FLOAT_MAT4:        return 16;
        default:                   return 0;  // a sampler or an image keeps the unit it is bound to
    }
}

} // namespace

void ShaderBase::listPlainUniforms() const {
    finishLink();
    m_plain.clear();
    GLint active = 0;
    VKM_GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &active));
    size_t most = 0;
    for (GLint i = 0; i < active; ++i) {
        char   name[256];
        GLint  count = 0;
        GLenum type  = 0;
        const auto index = static_cast<GLuint>(i);
        VKM_GL_CHECK(glGetActiveUniform(m_id, index, sizeof(name), nullptr, &count, &type, name));
        // A block's member has no location.
        GLint location = -1;
        VKM_GL_CHECK(location = glGetUniformLocation(m_id, name));
        const uint32_t values = valuesIn(type);
        if (location < 0 || values == 0) continue;
        m_plain.push_back(PlainUniform{location, count, type});
        most = std::max<size_t>(most, static_cast<size_t>(count) * values);
    }
    m_zeros.assign(most, 0.0f);
    m_intZeros.assign(most, 0u);
    m_plainListed = true;
}

void ShaderBase::zeroUniforms() const {
    if (!m_plainListed) listPlainUniforms();

    const GLfloat* zeros = m_zeros.data();
    const auto*    ints  = reinterpret_cast<const GLint*>(m_intZeros.data());
    const GLuint*  uints = m_intZeros.data();
    for (const PlainUniform& u : m_plain) {
        const GLint   at = u.location;
        const GLsizei n  = u.count;
        switch (u.type) {
            case GL_FLOAT:             VKM_GL_CHECK(glProgramUniform1fv(m_id, at, n, zeros)); break;
            case GL_FLOAT_VEC2:        VKM_GL_CHECK(glProgramUniform2fv(m_id, at, n, zeros)); break;
            case GL_FLOAT_VEC3:        VKM_GL_CHECK(glProgramUniform3fv(m_id, at, n, zeros)); break;
            case GL_FLOAT_VEC4:        VKM_GL_CHECK(glProgramUniform4fv(m_id, at, n, zeros)); break;
            case GL_FLOAT_MAT3:
                VKM_GL_CHECK(glProgramUniformMatrix3fv(m_id, at, n, GL_FALSE, zeros));
                break;
            case GL_FLOAT_MAT4:
                VKM_GL_CHECK(glProgramUniformMatrix4fv(m_id, at, n, GL_FALSE, zeros));
                break;
            case GL_INT:
            case GL_BOOL:              VKM_GL_CHECK(glProgramUniform1iv(m_id, at, n, ints)); break;
            case GL_INT_VEC2:
            case GL_BOOL_VEC2:         VKM_GL_CHECK(glProgramUniform2iv(m_id, at, n, ints)); break;
            case GL_INT_VEC3:
            case GL_BOOL_VEC3:         VKM_GL_CHECK(glProgramUniform3iv(m_id, at, n, ints)); break;
            case GL_INT_VEC4:
            case GL_BOOL_VEC4:         VKM_GL_CHECK(glProgramUniform4iv(m_id, at, n, ints)); break;
            case GL_UNSIGNED_INT:      VKM_GL_CHECK(glProgramUniform1uiv(m_id, at, n, uints)); break;
            case GL_UNSIGNED_INT_VEC2: VKM_GL_CHECK(glProgramUniform2uiv(m_id, at, n, uints)); break;
            case GL_UNSIGNED_INT_VEC3: VKM_GL_CHECK(glProgramUniform3uiv(m_id, at, n, uints)); break;
            case GL_UNSIGNED_INT_VEC4: VKM_GL_CHECK(glProgramUniform4uiv(m_id, at, n, uints)); break;
            default: break;
        }
    }
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
        VKM_GL_CHECK(glProgramUniform4f(m_id, location, f0, f1, f2, f3));
    }
}

void ShaderBase::setUniform3f(const char* uniformName, float f0, float f1, float f2) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform3f(m_id, location, f0, f1, f2));
    }
}

void ShaderBase::setUniform2f(const char* uniformName, float f0, float f1) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform2f(m_id, location, f0, f1));
    }
}

void ShaderBase::setUniform1f(const char* uniformName, float f0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform1f(m_id, location, f0));
    }
}

void ShaderBase::setUniform4fv(const char* uniformName, const glm::vec4& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform4fv(m_id, location, 1, &v[0]));
    }
}

void ShaderBase::setUniform3fv(const char* uniformName, const glm::vec3& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform3fv(m_id, location, 1, &v[0]));
    }
}

void ShaderBase::setUniform2i(const char* uniformName, int32_t i0, int32_t i1) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform2i(m_id, location, i0, i1));
    }
}

void ShaderBase::setUniform2fv(const char* uniformName, const glm::vec2& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform2fv(m_id, location, 1, &v[0]));
    }
}

void ShaderBase::setUniformMatrix4fv(const char* uniformName, const glm::mat4& matrix) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniformMatrix4fv(m_id, location, 1, GL_FALSE, &matrix[0][0]));
    }
}

void ShaderBase::setUniformMatrix3fv(const char* uniformName, const glm::mat3& matrix) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniformMatrix3fv(m_id, location, 1, GL_FALSE, &matrix[0][0]));
    }
}

void ShaderBase::setUniform1ui(const char* uniformName, uint32_t ui0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform1ui(m_id, location, ui0));
    }
}

void ShaderBase::setUniform1i(const char* uniformName, int32_t i0) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform1i(m_id, location, i0));
    }
}

void ShaderBase::setUniform3iv(const char* uniformName, const glm::ivec3& v) const {
    const int32_t location = getUniformLocation(uniformName);
    if (location != -1) {
        VKM_GL_CHECK(glProgramUniform3iv(m_id, location, 1, &v[0]));
    }
}

} // namespace Vkm::GL
