#pragma once

#include <string>

namespace Vkm::GL {

/**
 * @brief GLSL version injected when none is set explicitly - GL 4.3 core.
 */
constexpr int DEFAULT_GLSL_VERSION = 430;

/**
 * @brief Set the text prepended to every shader stage.
 *
 * Two things go in front of every stage and both are the same idea - something
 * the loader owns rather than something each file copies. The `#version` comes
 * from the GL context version requested at window creation. The constants come
 * from whoever is driving this: array capacities, grid dimensions, debug-mode
 * ordinals - values that live in the application's own code and would otherwise
 * be hand-copied into GLSL, or generated into it by a build step that has to
 * re-read the C++ to find them.
 *
 * Applies to graphics and compute stages alike, since both go through
 * preprocessShaderSource. Call once at startup, before any shader is built.
 *
 * @param glslVersion Version in GLSL's integer form, e.g. 430 for 4.3.
 * @param constants GLSL declarations inserted after the `#version` line;
 *                  empty for an application that needs none.
 */
void setShaderPrelude(int glslVersion, std::string constants = {});

/**
 * @brief Load a shader stage from disk with `#version` prepended and
 *        `#include "rel.glsl"` directives resolved.
 *
 * GLSL has neither natively. Includes are inlined relative to the including
 * file's own directory and are cycle-safe; the directive itself is preserved as
 * a comment so compile errors inside an inlined body stay navigable. A stage
 * with no `#include` is returned verbatim.
 *
 * This is the single loader for every stage - graphics and compute alike - so
 * both share include support and one version source of truth.
 *
 * @param filePath Path to the shader stage file on disk.
 * @return The stage source, ready to compile.
 */
std::string preprocessShaderSource(const std::string& filePath);

} // namespace Vkm::GL
