#pragma once

#include <cstdint>
#include <string>

#include "gl_shader_base.h"

namespace Core {

/**
 * @brief Holds the source code for a compute shader stage.
 *
 * Can be constructed with a path to automatically load compute shader source file from disk.
 * Validates compute shader source file presence.
 */
struct ComputeShaderSource {
    ComputeShaderSource();
    ~ComputeShaderSource() = default;

    explicit ComputeShaderSource(const std::string& path);

    /**
     * @brief Check existence of the compute shader source file.
     * @param computeShaderName Full path to the compute shader file.
     * @return True if path is valid, false otherwise.
     */
    bool validatePath(const std::string& computeShaderName);

    std::string m_path;           ///< Path to the containing directory of shader file
    std::string computeShader;    ///< Source code for compute shader
};

/**
 * @brief OpenGL compute shader program wrapper.
 *
 * Handles compilation, linkage, binding/unbinding, and uniform value setting for compute shaders.
 * Automatically loads compute shader from directory containing computeShader.shader.
 *
 * Provides compute-specific functionality like dispatch() for executing compute operations.
 *
 * Non-copyable, movable (inherits move semantics from ShaderBase).
 */
class ComputeShader : public ShaderBase {
    public:
        ComputeShader() = delete;
        ~ComputeShader() override = default;

        ComputeShader(const ComputeShader& shader) = delete;
        ComputeShader& operator=(const ComputeShader& shader) = delete;

        ComputeShader(ComputeShader && shader) noexcept = default;
        ComputeShader& operator=(ComputeShader && shader) noexcept = default;

        /**
         * @brief Construct and load compute shader from the directory at path.
         * @param path Path to the directory containing computeShader.shader.
         * @throws std::runtime_error if shader source validation or compilation fails.
         */
        explicit ComputeShader(const std::string& path);

        /**
         * @brief Dispatch compute shader with specified work group sizes.
         * @param numGroupsX Number of work groups in X dimension.
         * @param numGroupsY Number of work groups in Y dimension.
         * @param numGroupsZ Number of work groups in Z dimension.
         */
        void dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1) const;

    protected:
        /**
         * @brief Creates the compute shader program.
         */
        void createProgram() override;

        /**
         * @brief Reloads shader sources from disk.
         */
        void reloadSource() override;

    private:
        ComputeShaderSource m_source;
};

};
