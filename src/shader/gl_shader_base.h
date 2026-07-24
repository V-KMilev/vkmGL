#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
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

        // Uniform setters. The cache is content-hashed; any source of the name
        // works (string literal, std::string::c_str(), runtime-constructed).
        void setUniform4f(const char* uniformName, float f0, float f1, float f2, float f3) const;
        void setUniform3f(const char* uniformName, float f0, float f1, float f2) const;
        void setUniform2f(const char* uniformName, float f0, float f1) const;
        void setUniform1f(const char* uniformName, float f0) const;

        void setUniform4fv(const char* uniformName, const glm::vec4& v) const;
        void setUniform3fv(const char* uniformName, const glm::vec3& v) const;
        void setUniform2fv(const char* uniformName, const glm::vec2& v) const;

        void setUniformMatrix4fv(const char* uniformName, const glm::mat4& matrix) const;
        void setUniformMatrix3fv(const char* uniformName, const glm::mat3& matrix) const;

        void setUniform1ui(const char* uniformName, uint32_t ui0) const;
        void setUniform1i(const char* uniformName, int32_t i0) const;
        void setUniform3iv(const char* uniformName, const glm::ivec3& v) const;

        /**
         * @brief Quietly query whether the linked program has @p name as an
         *        active uniform.
         *
         * Same lookup + cache as getUniformLocation but does NOT log a
         * warning on miss. Use when "absent" is expected - shader variants
         * legitimately strip samplers/uniforms whose feature wasn't enabled,
         * and the caller wants to skip the bind silently instead of being
         * spammed by the strict warning.
         */
        bool hasUniform(const char* name) const;

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
         * @brief Get the uniform location for a uniform variable (content-cached).
         *
         * @param name Uniform name in shader.
         * @return Location, or -1 if not found.
         */
        int32_t getUniformLocation(const char* name) const;

    protected:
        std::string m_name;
        std::string m_path;

        /// Owns the chars backing the cache keys below. A std::deque keeps
        /// existing elements at stable addresses as it grows, so the
        /// string_view keys never dangle.
        mutable std::deque<std::string> m_uniformNames;

        /// Uniform-location cache keyed on string_view into m_uniformNames, so
        /// a const char* / literal lookup hashes in place with no allocation.
        /// (A std::string key would construct + hash a temporary every call,
        /// heap-allocating for names past the small-string buffer - and C++17
        /// has no heterogeneous unordered_map lookup to avoid it.)
        mutable std::unordered_map<std::string_view, int32_t> m_uniformLocationCache;

    private:
        /// Shared body of hasUniform()/getUniformLocation(): cache probe,
        /// glGetUniformLocation on miss, then own the name + cache the result.
        /// warnIfMissing logs the not-found case (getUniformLocation) or stays
        /// quiet (hasUniform).
        int32_t resolveUniform(const char* name, bool warnIfMissing) const;

        /// Delete the GL program and zero m_id. Idempotent — safe on a
        /// moved-from shader or after a previous release().
        void release() noexcept;
};

};
