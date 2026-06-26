#pragma once

#include <cstdint>
#include <string>

#include "gl_shader_base.h"

namespace Core {

/**
 * @brief Set the GLSL `#version` the loader injects atop every graphics stage.
 *
 * Graphics shader files omit their own `#version`; the loader prepends
 * "#version <v> core" so the value lives in one place (the GL context version
 * requested at window creation) instead of being copied into every shader. Call
 * once at startup, before any Shader is constructed. Defaults to 430.
 */
void setGraphicsShaderVersion(int glslVersion);

/**
 * @brief Holds the source code for graphics shader stages (vertex, fragment, geometry).
 *
 * Can be constructed with a path to automatically load shader source files from disk.
 * Validates shader source file presence. Geometry shader is optional.
 */
struct GraphicsShaderSource {
    GraphicsShaderSource();
    ~GraphicsShaderSource() = default;

    /**
     * @brief Load shader source files from a directory, resolving `#include`.
     *
     * Reads vertex.shader, fragment.shader, and optionally geometry.shader,
     * running each through preprocessShaderSource so `#include "rel.glsl"`
     * directives are inlined (relative to the including file). A stage with no
     * #include is read verbatim. For other preprocessing (e.g. injecting variant
     * #defines) construct the struct directly with prepared source strings.
     */
    explicit GraphicsShaderSource(const std::string& path);

    /**
     * @brief Direct construction with already-prepared source strings.
     *
     * Used when the caller has run its own preprocessing (e.g., resolved
     * `#include` directives) and wants to hand the final source straight
     * to compilation, bypassing the from-disk file reads.
     *
     * @param path Logical directory the source was loaded from. Used only
     *             for diagnostics and hot-reload reloadSource() lookups.
     * @param vertexShader   Final vertex shader source string.
     * @param fragmentShader Final fragment shader source string.
     * @param geometryShader Optional geometry shader source (empty = none).
     */
    GraphicsShaderSource(std::string path,
                         std::string vertexShader,
                         std::string fragmentShader,
                         std::string geometryShader = {});

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
 * Automatically loads shaders from directory containing vertex.shader, fragment.shader,
 * and optionally geometry.shader.
 *
 * Non-copyable, movable (inherits move semantics from ShaderBase).
 */
class Shader : public ShaderBase {
    public:
        Shader() = delete;
        ~Shader() override = default;

        Shader(const Shader& shader) = delete;
        Shader& operator=(const Shader& shader) = delete;

        Shader(Shader && shader) noexcept = default;
        Shader& operator=(Shader && shader) noexcept = default;

        /**
         * @brief Construct and load graphics shaders from the directory at path.
         * @param path Path to the directory containing vertex.shader, fragment.shader, and optionally geometry.shader.
         * @throws std::runtime_error if shader source validation or compilation fails.
         */
        explicit Shader(const std::string& path);

        /**
         * @brief Construct from already-prepared source strings.
         *
         * Skips the from-disk file read; the source is taken as-is. The
         * @p path is still recorded so reloadSource() and diagnostics work;
         * a derived class is expected to override reloadSource() if hot
         * reload should re-run the caller's preprocessor.
         */
        explicit Shader(GraphicsShaderSource source);

    protected:
        /**
         * @brief Creates the graphics shader program (vertex + fragment + optional geometry).
         */
        void createProgram() override;

        /**
         * @brief Reloads shader sources from disk.
         */
        void reloadSource() override;

    protected:
        /// Visible to subclasses so they can override reloadSource() and
        /// install a freshly-prepared source (e.g. re-running an include
        /// preprocessor) without going through the base path-based loader.
        GraphicsShaderSource m_source;
};

};
