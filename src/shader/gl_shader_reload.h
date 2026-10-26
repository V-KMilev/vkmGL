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
 * itself here on construction, hands its place over when moved and removes
 * itself on destruction, which keeps that list correct without every call site
 * opting in. The same list says how many are still compiling (countShaders())
 * and checks them all (checkShaders()).
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
 * @brief Give @p to the place @p from held in the live set.
 *
 * What a move takes is the program, and the place goes with it: @p to is live exactly when
 * @p from was, in its order, and @p from is live no more. So a moved-from shader is never walked.
 *
 * @param from The shader moved from.
 * @param to   The shader moved into.
 */
void handOverShader(ShaderBase* from, ShaderBase* to);

/// How many shaders are live, and how many of them the driver is still compiling.
struct ShaderCount {
    uint32_t live      = 0;
    uint32_t compiling = 0;  ///< Not ShaderBase::ready().
};

/**
 * @brief Count the live shaders, asking each whether it is ready without waiting for it.
 *
 * For a host showing progress while ShaderBase::compileInParallel() builds them.
 *
 * @return The counts.
 */
ShaderCount countShaders();

/**
 * @brief Check every live shader's compile and link, waiting for any still building.
 *
 * With ShaderBase::compileInParallel() on, a program is otherwise checked at its first use, which
 * can come long after startup. A host calls this once countShaders() finds none compiling, so a
 * failure surfaces then. A shader already checked costs nothing.
 *
 * @throws std::runtime_error At the first shader that failed to compile or link, having logged it.
 */
void checkShaders();

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
