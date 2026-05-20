#include "gl_shader.h"

#include <filesystem>
#include <stdexcept>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

#include "file_read.h"

namespace fs = std::filesystem;

namespace Core {

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
    const std::string vertexShaderName = (fs::path(m_path) / "vertexShader.shader").string();
    const std::string fragmentShaderName = (fs::path(m_path) / "fragmentShader.shader").string();
    std::string geometryShaderName = (fs::path(m_path) / "geometryShader.shader").string();

    if (!validatePaths(
        vertexShaderName,
        fragmentShaderName,
        geometryShaderName
    )) {
        throw std::runtime_error("Graphics shader source validation failed");
    }

    vertexShader = fileToString(vertexShaderName);
    fragmentShader = fileToString(fragmentShaderName);

    if (!geometryShaderName.empty()) {
        geometryShader = fileToString(geometryShaderName);
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
        LOG_WARNING("Graphics shader '%s' running without geometry shader", m_path.c_str());
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
