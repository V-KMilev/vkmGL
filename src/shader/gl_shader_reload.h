#pragma once

#include <cstdint>
#include <string>

namespace Vkm::GL {

class ShaderBase;

/**
 * @brief Development-time shader hot reload.
 *
 * Shaders are constructed all over the renderer - one per pass, plus the
 * bakers and the editor's preview - and nothing owns them collectively, so
 * there is no list to walk when a source file changes. Every ShaderBase adds
 * itself here on construction and removes itself on destruction, which keeps
 * that list correct without every call site opting in.
 *
 * Change detection is deliberately coarse: reloadChangedShaders() takes the
 * newest write time anywhere under the watched directory and recompiles
 * everything when it moves. Tracking per-shader file dependencies would be
 * finer, but shaders #include shared headers (a lighting or shadow helper is
 * pulled into most programs), so a precise implementation would end up
 * recompiling most of them anyway. Recompiling all of them takes long enough to
 * notice once and not long enough to care, and it cannot miss a dependency.
 *
 * Not thread-safe, and not meant to be: registration and reload both happen on
 * the thread that owns the GL context.
 */

/**
 * @brief Add @p shader to the live set. Called by ShaderBase's constructor.
 */
void registerShader(ShaderBase* shader);

/**
 * @brief Remove @p shader from the live set. Called by ShaderBase's destructor.
 */
void unregisterShader(ShaderBase* shader);

/**
 * @brief Recompile every live shader if anything under @p directory changed.
 *
 * Cheap to call every frame: the common path is one directory walk and a
 * timestamp comparison. A shader whose source no longer compiles keeps its
 * previous program and logs the error, so a syntax error costs a red line in
 * the log rather than a broken renderer.
 *
 * @param directory Root to watch, e.g. "shaders".
 * @return Number of shaders recompiled; 0 when nothing changed.
 */
uint32_t reloadChangedShaders(const std::string& directory);

} // namespace Vkm::GL
