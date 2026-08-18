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

namespace VkmGL {

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
         */
        void bind() const;

        /**
         * @brief Unbind any currently bound shader program (glUseProgram(0)).
         */
        void unbind() const;

        /**
         * @brief Reload shader sources from disk and recreate the program.
         * @throws std::runtime_error on compilation or linking failure.
         */
        void recompile();

        /**
         * @brief Reload and recompile, keeping the working program on failure.
         *
         * recompile() releases the live program before building the new one, so
         * a source file that no longer compiles leaves the shader with nothing
         * and throws. That is the right behaviour at startup, where a broken
         * shader should stop the program, and the wrong one for hot reload,
         * where a half-typed edit must not take the renderer down with it.
         *
         * Builds the replacement first and adopts it only once it has linked.
         * On failure the previous program stays bound-able and the error is
         * logged, so the next save can fix it.
         *
         * @return True if the new program linked and is now in use.
         */
        bool tryRecompile();

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
        void setUniform2i(const char* uniformName, int32_t i0, int32_t i1) const;
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
         * @brief Log a stage's compilation error, delete the stage, and throw.
         *
         * Never returns - the name says so, because a caller reading
         * `printCompilationError(...)` would reasonably expect control to
         * continue past it.
         *
         * @param type OpenGL shader stage enum.
         * @param id   Shader object ID (deleted before throwing).
         * @throws std::runtime_error Always.
         */
        [[noreturn]] void throwCompilationError(uint32_t type, uint32_t id) const;

        /**
         * @brief Get the uniform location for a uniform variable (content-cached).
         *
         * @param name Uniform name in shader.
         * @return Location, or -1 if not found.
         */
        int32_t getUniformLocation(const char* name) const;

    private:
        /**
         * @brief Shared body of hasUniform() and getUniformLocation().
         *
         * Probes the cache, calls glGetUniformLocation on a miss, then takes
         * ownership of the name so the cache's view key cannot dangle.
         *
         * @param name          Uniform name to resolve.
         * @param warnIfMissing Log the not-found case (getUniformLocation), or
         *                      stay quiet for the expected-absent one
         *                      (hasUniform).
         * @return The location, or -1 when the program has no such active uniform.
         */
        int32_t resolveUniform(const char* name, bool warnIfMissing) const;

        /**
         * @brief Delete the GL program and zero m_id.
         *
         * Idempotent, so it is safe on a moved-from shader or after a previous
         * release().
         */
        void release() noexcept;

    protected:
        std::string m_name;
        std::string m_path;

        /// Backs the cache keys; a deque's addresses stay put as it grows.
        mutable std::deque<std::string> m_uniformNames;

        /// Name -> location, keyed by view so a literal hashes without allocating.
        mutable std::unordered_map<std::string_view, int32_t> m_uniformLocationCache;
};

} // namespace VkmGL
