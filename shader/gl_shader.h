#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "gl_object.h"

namespace Core {

/**
 * @brief Holds the source code for different shader stages (vertex, fragment, geometry)
 *        and the path to the directory containing the shader files.
 *
 * Can be constructed with a path to automatically load shader source files from disk.
 * Provides validation of shader source file presence and assigns an empty geometry shader if missing.
 */
struct ShaderSource {
    ShaderSource();
    ~ShaderSource() = default;

    explicit ShaderSource(const std::string& path);

    /**
     * @brief Check existence of the shader source files.
     *        If vertex/fragment is missing, returns false. Geometry is optional; if missing, returns true and clears the path.
     *
     * @param vertexShaderName   Full path to the vertex shader file.
     * @param fragmentShaderName Full path to the fragment shader file.
     * @param geometryShaderName Full path to the geometry shader file (modified if file missing).
     * @return True if all paths are valid, false otherwise.
     */
    bool validatePaths(
        const std::string& vertexShaderName,
        const std::string& fragmentShaderName,
        std::string& geometryShaderName
    );

    std::string m_path;           ///< Path to the containing directory of shader files (vertex/fragment/geometry)
    std::string vertexShader;     ///< Source code for vertex shader
    std::string fragmentShader;   ///< Source code for fragment shader
    std::string geometryShader;   ///< Source code for geometry shader
};

/**
 * @brief OpenGL Shader program wrapper.
 *
 * Handles compilation, linkage, binding/unbinding, uniform value setting, and program recreation.
 * Disallows copy and move.
 */
class Shader : public GLObject {
    public:
        Shader() = delete;
        ~Shader() override;

        Shader(const Shader& shader) = delete;
        Shader& operator=(const Shader& shader) = delete;

        Shader(Shader && shader) = delete;
        Shader& operator=(Shader && shader) = delete;

        /**
         * @brief Construct and load shaders from the directory at path.
         * @param path Path to the directory containing shader files (vertex/fragment/geometry).
         * @throws std::runtime_error if shader source validation fails.
         */
        explicit Shader(const std::string& path);

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
         * @brief Reload shader sources from disk and recreate the program (throws on failure).
         */
        void reCompleShader();

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

    private:
        /**
         * @brief Compiles and links shader stages, then creates an OpenGL program.
         */
        void createShader();

        /**
         * @brief Compiles a single shader stage.
         * @param type OpenGL shader stage enum (e.g. GL_VERTEX_SHADER).
         * @param source Shader source code string.
         * @return Compiled shader object ID.
         */
        uint32_t compileShader(uint32_t type, const std::string& source) const;

        /**
         * @brief Prints the compilation errors for a shader and throws.
         * @param type OpenGL shader stage enum.
         * @param id   Shader object ID.
         * @return Will throw, but signature returns uint32_t for internal reasons.
         */
        uint32_t printError(uint32_t type, uint32_t id) const;

        /**
         * @brief Get the uniform location for a uniform variable (cached).
         * @param name Uniform name in shader.
         * @return Location, or -1 if not found.
         */
        int32_t getUniformLocation(const std::string& name) const;

    private:
        std::string m_name;
        std::string m_path;

        ShaderSource m_source;

        mutable std::unordered_map<std::string, int32_t> m_uniformLocationCache;
};
};
