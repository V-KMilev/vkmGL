#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "SHADER"

#include "gl_shader.h"

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "gl_error_handle.h"
#include "gl_shader_preprocess.h"
#include "l_assert.h"
#include "logger.h"

namespace fs = std::filesystem;

namespace Vkm::GL {

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
    VKM_GL_CHECK(m_id = glCreateProgram());
    VKM_ASSERT(m_id != 0);

    // Stage objects are released on every exit path. compileShader throws when
    // a stage fails, and the stages already built before it must not leak out
    // with the exception - a hot reload of a broken shader would otherwise leak
    // one object per save.
    std::vector<uint32_t> stages;
    const auto deleteStages = [&stages]() {
        for (const uint32_t stage : stages) {
            VKM_GL_CHECK(glDeleteShader(stage));
        }
    };

    try {
        stages.push_back(compileShader(GL_VERTEX_SHADER, m_source.vertexShader));
        stages.push_back(compileShader(GL_FRAGMENT_SHADER, m_source.fragmentShader));

        if (!m_source.geometryShader.empty()) {
            stages.push_back(compileShader(GL_GEOMETRY_SHADER, m_source.geometryShader));
        }

        for (const uint32_t stage : stages) {
            VKM_GL_CHECK(glAttachShader(m_id, stage));
        }

        linkProgram(m_id);
    } catch (...) {
        deleteStages();
        throw;
    }

    // Apply a debug label for easier GPU debugging.
    setLabel(m_name.c_str());

    deleteStages();

    LOG_TRACE("Graphics shader '%s' successfully created", m_path.c_str());
}

} // namespace Vkm::GL
