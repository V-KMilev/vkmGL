#pragma once

#include <string>

namespace VkmGL {

/**
 * @brief GLSL version injected when none is set explicitly - GL 4.3 core.
 */
constexpr int DEFAULT_GLSL_VERSION = 430;

/**
 * @brief Set the GLSL `#version` prepended to every shader stage.
 *
 * Shader files omit their own `#version` so the value lives in one place - the
 * GL context version requested at window creation - instead of being copied
 * into every file. Applies to graphics and compute stages alike, since both go
 * through preprocessShaderSource. Call once at startup, before any shader is
 * constructed.
 *
 * @param glslVersion Version in GLSL's integer form, e.g. 430 for 4.3.
 */
void setShaderVersion(int glslVersion);

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

} // namespace VkmGL
