#define VKM_LOG_SUFFIX "VKM-GL"
#define VKM_LOG_CATEGORY "SHADER"

#include "gl_compute_shader.h"

#include <filesystem>
#include <stdexcept>

#include "gl_error_handle.h"
#include "l_assert.h"
#include "logger.h"

#include "gl_shader_preprocess.h"

namespace fs = std::filesystem;

namespace VkmGL {

ComputeShaderSource::ComputeShaderSource()
    : m_path()
    , computeShader() {}

ComputeShaderSource::ComputeShaderSource(const std::string& path)
    : m_path(path)
    , computeShader() {
    const std::string computeShaderName = (fs::path(m_path) / "computeShader.shader").string();

    if (!validatePath(computeShaderName)) {
        throw std::runtime_error("Compute shader source validation failed");
    }

    // Same loader as graphics stages: prepend #version and resolve #include, so
    // compute shaders share the engine's generated constants (engine_config.glsl).
    computeShader = preprocessShaderSource(computeShaderName);
}

bool ComputeShaderSource::validatePath(const std::string& computeShaderName) {
    if (!fs::exists(computeShaderName)) {
        LOG_FATAL("Compute shader '%s' missing compute shader at '%s'", m_path.c_str(), computeShaderName.c_str());
        return false;
    }
    return true;
}

ComputeShader::ComputeShader(const std::string& path)
    : ShaderBase(path)
    , m_source(path) {
    createProgram();
}

void ComputeShader::reloadSource() {
    m_source = ComputeShaderSource(m_path);
}

void ComputeShader::createProgram() {
    VKM_GL_CHECK(m_id = glCreateProgram());
    VKM_ASSERT(m_id != 0);

    const uint32_t computeShader = compileShader(GL_COMPUTE_SHADER, m_source.computeShader);
    VKM_GL_CHECK(glAttachShader(m_id, computeShader));

    linkProgram(m_id);

    // Apply a debug label for easier GPU debugging.
    setLabel(m_name.c_str());

    VKM_GL_CHECK(glDeleteShader(computeShader));

    LOG_TRACE("Compute shader '%s' successfully created", m_path.c_str());
}

void ComputeShader::dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) const {
    VKM_GL_CHECK(glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
}

} // namespace VkmGL
