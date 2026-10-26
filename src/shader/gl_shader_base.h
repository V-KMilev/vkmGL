#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "gl_object.h"

namespace Vkm::GL {

/**
 * @brief Base class for OpenGL shader programs.
 *
 * Provides common functionality for all shader types:
 * - Program binding/unbinding
 * - Uniform value setting (all types)
 * - Shader compilation and linking, in the driver's threads where it has them
 *   (compileInParallel()), checked at the program's first use or checkShaders()
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

        ShaderBase(ShaderBase && shader) noexcept;
        ShaderBase& operator=(ShaderBase && shader) noexcept;

    public:
        /**
         * @brief Let the driver compile and link programs on threads of its own, if it can.
         *
         * Called once the context is current, before the programs it should speed. A program
         * built while it is on is handed to the driver and checked at its first use, which waits
         * for it; ready() says whether that would wait. While it is off, or on a driver without
         * GL_KHR_parallel_shader_compile or GL_ARB_parallel_shader_compile, a program is checked
         * as it is built.
         *
         * @param on Whether to; off gives the driver's threads back.
         * @return Whether the driver now compiles in parallel.
         */
        static bool compileInParallel(bool on = true);

        /**
         * @brief Whether the program has compiled and linked, asked without waiting for it.
         *
         * @return True once the driver is done with it, failed or not; a failure throws at
         *         the program's first use, or at checkShaders().
         */
        bool ready() const;

        /**
         * @brief Bind this shader program (glUseProgram).
         */
        void bind() const;

        /**
         * @brief Unbind any currently bound shader program (glUseProgram(0)).
         */
        void unbind() const;

        /**
         * @brief Reload the sources from disk, keeping the working program on failure.
         *
         * Builds the replacement first and adopts it only once it has linked, so
         * a half-typed edit does not take the renderer down with it: the previous
         * program stays bound-able and the error is logged for the next save to
         * fix. Construction is the opposite case and behaves the opposite way -
         * a shader that will not compile at startup throws, because a program
         * that cannot draw should not start.
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

        // Uniform setters, on this program whether it is bound or not. The cache is content-hashed;
        // any source of the name works (string literal, std::string::c_str(), runtime-constructed).
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

        /**
         * @brief Set every uniform outside a block that is a float, int, uint or bool scalar or
         *        vector, a mat3 or a mat4, to zero.
         *
         * A program shared by passes that each set only some starts from this rather than from
         * what another left. Any other type is left as it is: samplers and images keep their
         * units, and a mat2, a non-square matrix or a double keeps its value. A block is its
         * buffer's. The uniforms are listed once a link.
         */
        void zeroUniforms() const;

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
         * @brief Start compiling one stage, without waiting for or checking the result.
         *
         * @param type   OpenGL shader stage enum (e.g. GL_VERTEX_SHADER, GL_COMPUTE_SHADER).
         * @param source Shader source code string.
         * @return The stage's shader object ID.
         */
        uint32_t startStage(uint32_t type, const std::string& source) const;

        /**
         * @brief Attach @p stages to the program and start linking it.
         *
         * The program keeps the stages until it is checked: at once without
         * compileInParallel(), else at its first use.
         *
         * @param stages The program's stages, from startStage().
         * @throws std::runtime_error When checked at once and a stage or the link failed.
         */
        void startLink(std::vector<uint32_t> stages);

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
        /// A uniform zeroUniforms() sets.
        struct PlainUniform {
            int32_t  location = -1;
            int32_t  count    = 0;  ///< Its array's length, 1 for a single value.
            uint32_t type     = 0;  ///< Its GL type.
        };

        /**
         * @brief Wait for the program's compile and link, check them, and free its stages.
         *
         * Nothing once checked. A failure logs the driver's message, frees the stages and
         * throws; the program stays, unusable, for its owner to drop.
         *
         * @throws std::runtime_error When a stage failed to compile or the program to link.
         */
        void finishLink() const;

        // Runs finishLink() on every live shader.
        friend void checkShaders();

        /// List the program's plain uniforms into m_plain, and size the zeros for the largest.
        void listPlainUniforms() const;

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

        // What zeroUniforms() sets, listed on first use after a link, and zeros for the largest.
        mutable std::vector<PlainUniform> m_plain;
        mutable bool                      m_plainListed = false;
        mutable std::vector<float>        m_zeros;
        mutable std::vector<uint32_t>     m_intZeros;  ///< Read as signed and unsigned alike.

        /// The stages of a program not yet checked; empty once finishLink() ran.
        mutable std::vector<uint32_t> m_stages;
};

} // namespace Vkm::GL
