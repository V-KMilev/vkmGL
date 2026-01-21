#pragma once

#include <cstdint>
#include <string>

#include "gl_shader_base.h"

namespace Core {

/**
 * @brief Holds the source code for graphics shader stages (vertex, fragment, geometry).
 *
 * Can be constructed with a path to automatically load shader source files from disk.
 * Validates shader source file presence. Geometry shader is optional.
 */
struct GraphicsShaderSource {
    GraphicsShaderSource();
    ~GraphicsShaderSource() = default;

    explicit GraphicsShaderSource(const std::string& path);

    /**
     * @brief Check existence of the shader source files.
     *        If vertex/fragment is missing, returns false. Geometry is optional; if missing, returns true and clears the path.
     *
     * @param vertexShaderName   Full path to the vertex shader file.
     * @param fragmentShaderName Full path to the fragment shader file.
     * @param geometryShaderName Full path to the geometry shader file (modified if file missing).
     * @return True if all required paths are valid, false otherwise.
     */
    bool validatePaths(
        const std::string& vertexShaderName,
        const std::string& fragmentShaderName,
        std::string& geometryShaderName
    );

    std::string m_path;           ///< Path to the containing directory of shader files
    std::string vertexShader;     ///< Source code for vertex shader
    std::string fragmentShader;   ///< Source code for fragment shader
    std::string geometryShader;   ///< Source code for geometry shader (optional)
};

/**
 * @brief OpenGL graphics shader program wrapper (vertex + fragment + optional geometry).
 *
 * Handles compilation, linkage, binding/unbinding, and uniform value setting for graphics shaders.
 * Automatically loads shaders from directory containing vertexShader.shader, fragmentShader.shader,
 * and optionally geometryShader.shader.
 *
 * Disallows copy and move.
 */
class Shader : public ShaderBase {
    public:
        Shader() = delete;
        ~Shader() override = default;

        Shader(const Shader& shader) = delete;
        Shader& operator=(const Shader& shader) = delete;

        Shader(Shader && shader) = delete;
        Shader& operator=(Shader && shader) = delete;

        /**
         * @brief Construct and load graphics shaders from the directory at path.
         * @param path Path to the directory containing vertexShader.shader, fragmentShader.shader, and optionally geometryShader.shader.
         * @throws std::runtime_error if shader source validation or compilation fails.
         */
        explicit Shader(const std::string& path);

    protected:
        /**
         * @brief Creates the graphics shader program (vertex + fragment + optional geometry).
         */
        void createProgram() override;

        /**
         * @brief Reloads shader sources from disk.
         */
        void reloadSource() override;

    private:
        GraphicsShaderSource m_source;
};

};
