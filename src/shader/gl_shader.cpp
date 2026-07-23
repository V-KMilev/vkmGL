#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "SHADER"

#include "gl_shader.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

namespace fs = std::filesystem;

namespace Core {

namespace {

// GLSL `#version` injected at the top of every graphics stage (see
// setGraphicsShaderVersion). Defaults to 430 (GL 4.3); the application overrides
// it at startup from its requested GL context version - one source of truth.
int g_glslVersion = 430;

// Resolve `#include "relative/path"` directives in GLSL source (GLSL has none
// natively). Inlines referenced files relative to each including file's
// directory, cycle-safe via `visited`; the directive is preserved as a comment
// so compile errors in the inlined body stay navigable. A stage with no
// #include is returned verbatim. Used via preprocessShaderSource by both the
// graphics and compute shader sources.
std::string resolveIncludes(const fs::path& filePath, std::unordered_set<std::string>& visited) {
    std::error_code ec;
    const fs::path canonical = fs::weakly_canonical(filePath, ec);
    const std::string key = ec ? filePath.string() : canonical.string();
    if (!visited.insert(key).second) {
        return "// (skipped duplicate include: " + filePath.string() + ")\n";
    }

    std::ifstream in(filePath);
    if (!in) {
        LOG_ERROR("Shader preprocessor: cannot open '%s'", filePath.string().c_str());
        return "// (failed to open: " + filePath.string() + ")\n";
    }

    const fs::path basedir = filePath.parent_path();
    std::ostringstream out;
    std::string line;
    while (std::getline(in, line)) {
        // Only a leading `#include` token is a directive (not one in a comment).
        size_t firstNonWS = 0;
        while (firstNonWS < line.size() &&
               (line[firstNonWS] == ' ' || line[firstNonWS] == '\t')) {
            ++firstNonWS;
        }
        if (line.compare(firstNonWS, 8, "#include") == 0) {
            const size_t q1 = line.find('"', firstNonWS + 8);
            const size_t q2 = q1 != std::string::npos ? line.find('"', q1 + 1) : std::string::npos;
            if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                out << "// " << line << "\n";
                out << resolveIncludes(basedir / line.substr(q1 + 1, q2 - q1 - 1), visited);
                continue;
            }
            LOG_WARNING("Shader preprocessor: malformed #include in '%s': %s",
                filePath.string().c_str(), line.c_str());
        }
        out << line << "\n";
    }
    return out.str();
}

} // namespace

std::string preprocessShaderSource(const std::string& filePath) {
    std::unordered_set<std::string> visited;
    // Prepend the #version directive (shaders omit their own), then the resolved
    // source. #version must be the first line; #included files never carry one.
    return "#version " + std::to_string(g_glslVersion) + " core\n"
         + resolveIncludes(fs::path(filePath), visited);
}

void setGraphicsShaderVersion(int glslVersion) {
    g_glslVersion = glslVersion;
}

GraphicsShaderSource::GraphicsShaderSource()
    : m_path()
    , vertexShader()
    , fragmentShader()
    , geometryShader() {}

GraphicsShaderSource::GraphicsShaderSource(const std::string& path)
    : m_path(path)
    , vertexShader()
    , fragmentShader()
    , geometryShader() {
    const std::string vertexShaderName = (fs::path(m_path) / "vertex.shader").string();
    const std::string fragmentShaderName = (fs::path(m_path) / "fragment.shader").string();
    std::string geometryShaderName = (fs::path(m_path) / "geometry.shader").string();

    if (!validatePaths(
        vertexShaderName,
        fragmentShaderName,
        geometryShaderName
    )) {
        throw std::runtime_error("Graphics shader source validation failed");
    }

    // Resolve #include directives as each stage is loaded.
    vertexShader = preprocessShaderSource(vertexShaderName);
    fragmentShader = preprocessShaderSource(fragmentShaderName);

    if (!geometryShaderName.empty()) {
        geometryShader = preprocessShaderSource(geometryShaderName);
    }
}

GraphicsShaderSource::GraphicsShaderSource(std::string path,
                                           std::string vertexShader_,
                                           std::string fragmentShader_,
                                           std::string geometryShader_)
    : m_path(std::move(path))
    , vertexShader(std::move(vertexShader_))
    , fragmentShader(std::move(fragmentShader_))
    , geometryShader(std::move(geometryShader_)) {}

bool GraphicsShaderSource::validatePaths(
    const std::string& vertexShaderName,
    const std::string& fragmentShaderName,
    std::string& geometryShaderName
) {
    if (!fs::exists(vertexShaderName)) {
        LOG_FATAL("Graphics shader '%s' missing vertex shader at '%s'", m_path.c_str(), vertexShaderName.c_str());
        return false;
    }
    if (!fs::exists(fragmentShaderName)) {
        LOG_FATAL("Graphics shader '%s' missing fragment shader at '%s'", m_path.c_str(), fragmentShaderName.c_str());
        return false;
    }
    if (!fs::exists(geometryShaderName)) {
        // Normal operation for nearly every program - not a warning.
        LOG_TRACE("Graphics shader '%s' has no geometry stage", m_path.c_str());
        geometryShaderName.clear();
    }
    return true;
}

Shader::Shader(const std::string& path)
    : ShaderBase(path)
    , m_source(path) {
    createProgram();
}

Shader::Shader(GraphicsShaderSource source)
    : ShaderBase(source.m_path)
    , m_source(std::move(source)) {
    createProgram();
}

void Shader::reloadSource() {
    // Re-read from disk. Subclasses that bypass the path-based source (e.g.
    // engine-side preprocessors that resolve #include) override this to
    // re-run their own loader instead.
    m_source = GraphicsShaderSource(m_path);
}

void Shader::createProgram() {
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

    linkProgram(m_id);

    // Apply a debug label for easier GPU debugging.
    setLabel(m_name.c_str());

    VKM_GL_CHECK(glDeleteShader(vertexShader));
    VKM_GL_CHECK(glDeleteShader(fragmentShader));

    if (geometryShader != 0) {
        VKM_GL_CHECK(glDeleteShader(geometryShader));
    }

    LOG_TRACE("Graphics shader '%s' successfully created", m_path.c_str());
}

};
