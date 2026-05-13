#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "gl_object.h"

namespace Core {

/**
 * @brief Base class for OpenGL shader programs.
 *
 * Provides common functionality for all shader types:
 * - Program binding/unbinding
 * - Uniform value setting (all types)
 * - Shader compilation
 * - Program linking and validation
 * - Hot-reloading support
 * - Debug labeling
 *
 * Derived classes implement shader-type-specific program creation.
 */
class ShaderBase : public GLObject {
    public:
        ShaderBase() = delete;
        ~ShaderBase() override;

        ShaderBase(const ShaderBase& shader) = delete;
        ShaderBase& operator=(const ShaderBase& shader) = delete;

        ShaderBase(ShaderBase && shader) noexcept = default;
        ShaderBase& operator=(ShaderBase && shader) noexcept;

    public:
        /**
         * @brief Bind this shader program (glUseProgram).
         * @param target (Unused, defaults to GL_NONE)
         */
        void bind(GLenum target = GL_NONE) const override;

        /**
         * @brief Unbind any currently bound shader program (glUseProgram(0)).
         * @param target (Unused, defaults to GL_NONE)
         */
        void unbind(GLenum target = GL_NONE) const override;

        /**
         * @brief Reload shader sources from disk and recreate the program.
         * @throws std::runtime_error on compilation or linking failure.
         */
        void recompile();

        /**
         * @brief Get user-friendly shader name (the filename of the shader directory).
         * @return The name of the shader program.
         */
        const std::string& getName() const;

        /**
         * @brief Get the full path to the shader source directory.
         * @return The file path associated with this shader.
         */
        const std::string& getPath() const;

        // Uniform setters
        void setUniform4f(const std::string& uniformName, float f0, float f1, float f2, float f3) const;
        void setUniform3f(const std::string& uniformName, float f0, float f1, float f2) const;
        void setUniform2f(const std::string& uniformName, float f0, float f1) const;
        void setUniform1f(const std::string& uniformName, float f0) const;

        void setUniform4fv(const std::string& uniformName, const glm::vec4& v) const;
        void setUniform3fv(const std::string& uniformName, const glm::vec3& v) const;
        void setUniform2fv(const std::string& uniformName, const glm::vec2& v) const;

        void setUniformMatrix4fv(const std::string& uniformName, const glm::mat4& matrix) const;
        void setUniformMatrix3fv(const std::string& uniformName, const glm::mat3& matrix) const;

        void setUniform1ui(const std::string& uniformName, uint32_t ui0) const;
        void setUniform1i(const std::string& uniformName, int32_t i0) const;

    protected:
        /**
         * @brief Protected constructor for derived classes.
         * @param path Path to the directory containing shader files.
         */
        explicit ShaderBase(const std::string& path);

        /**
         * @brief Pure virtual method for derived classes to create their specific shader program.
         * Called during construction and recompilation.
         */
        virtual void createProgram() = 0;

        /**
         * @brief Pure virtual method for derived classes to reload their source files.
         */
        virtual void reloadSource() = 0;

        /**
         * @brief Compile a single shader stage.
         * @param type OpenGL shader stage enum (e.g. GL_VERTEX_SHADER, GL_COMPUTE_SHADER).
         * @param source Shader source code string.
         * @return Compiled shader object ID.
         * @throws std::runtime_error on compilation failure.
         */
        uint32_t compileShader(uint32_t type, const std::string& source) const;

        /**
         * @brief Link the shader program and validate it.
         * @param programId The OpenGL program ID to link.
         * @throws std::runtime_error on linking failure.
         */
        void linkProgram(uint32_t programId) const;

        /**
         * @brief Print compilation errors for a shader and throw.
         * @param type OpenGL shader stage enum.
         * @param id   Shader object ID.
         * @throws std::runtime_error Always throws after logging the error.
         */
        [[noreturn]] void printCompilationError(uint32_t type, uint32_t id) const;

        /**
         * @brief Get the uniform location for a uniform variable (cached).
         * @param name Uniform name in shader.
         * @return Location, or -1 if not found.
         */
        int32_t getUniformLocation(const std::string& name) const;

    protected:
        std::string m_name;
        std::string m_path;

        mutable std::unordered_map<std::string, int32_t> m_uniformLocationCache;

    private:
        /// Delete the GL program and zero m_id. Idempotent — safe on a
        /// moved-from shader or after a previous release().
        void release() noexcept;
};

};
