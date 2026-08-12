#include "gl_shader_reload.h"

#include <filesystem>
#include <optional>
#include <system_error>
#include <vector>

#include "logger.h"

#include "gl_shader_base.h"

namespace Core {

namespace {

// Live shaders, in construction order. A vector rather than a set because the
// list is short (tens), walked far more often than it is edited, and iteration
// order should be stable so two reload logs read the same way.
std::vector<ShaderBase*>& liveShaders() {
    static std::vector<ShaderBase*> shaders;
    return shaders;
}

// Newest write time seen under the watched directory; empty until the first
// poll, which establishes the baseline rather than reloading everything.
//
// Deliberately optional rather than a default-constructed sentinel. The epoch
// of std::filesystem::file_time_type is implementation-defined, and libstdc++
// places it in the future - so time_since_epoch() is NEGATIVE for present-day
// files and a default-constructed value compares GREATER than every real
// timestamp. Used as a sentinel it silently means "nothing is ever newer", and
// the reload never fires.
std::optional<std::filesystem::file_time_type>& lastSeenWrite() {
    static std::optional<std::filesystem::file_time_type> stamp;
    return stamp;
}

/**
 * @brief Newest last-write time anywhere under @p directory.
 *
 * @param directory Root to scan, recursively.
 * @return The newest time, or empty when the directory is missing, unreadable
 *         or holds no files - a packaged build with no shader sources on disk
 *         simply never reloads.
 */
std::optional<std::filesystem::file_time_type> newestWrite(const std::string& directory) {
    std::error_code ec;
    std::filesystem::recursive_directory_iterator it(directory, ec);
    if (ec) return std::nullopt;

    std::optional<std::filesystem::file_time_type> newest;
    for (const auto& entry : it) {
        std::error_code entryEc;
        if (!entry.is_regular_file(entryEc) || entryEc) continue;

        const auto written = entry.last_write_time(entryEc);
        if (entryEc) continue;

        if (!newest || written > *newest) newest = written;
    }
    return newest;
}

} // namespace

void registerShader(ShaderBase* shader) {
    if (shader) liveShaders().push_back(shader);
}

void unregisterShader(ShaderBase* shader) {
    std::vector<ShaderBase*>& shaders = liveShaders();
    for (size_t i = 0; i < shaders.size(); ++i) {
        if (shaders[i] != shader) continue;
        shaders.erase(shaders.begin() + static_cast<long>(i));
        return;
    }
}

uint32_t reloadChangedShaders(const std::string& directory) {
    const std::optional<std::filesystem::file_time_type> newest = newestWrite(directory);
    if (!newest) return 0;

    // The first poll only records where we started; reloading here would
    // recompile everything once at startup for no reason.
    if (!lastSeenWrite()) {
        lastSeenWrite() = newest;
        return 0;
    }

    if (*newest <= *lastSeenWrite()) return 0;
    lastSeenWrite() = newest;

    uint32_t reloaded = 0;
    uint32_t failed   = 0;
    for (ShaderBase* shader : liveShaders()) {
        // A moved-from shader stays registered until it is destroyed and has no
        // source left to read; skip it rather than reporting a failure for it.
        if (!shader || shader->getPath().empty()) continue;
        if (shader->tryRecompile()) ++reloaded;
        else                        ++failed;
    }

    if (failed > 0) {
        LOG_WARNING_C("SHADER", "Reloaded %u shader(s); %u kept their previous program",
                      reloaded, failed);
    } else {
        LOG_INFO_C("SHADER", "Reloaded %u shader(s)", reloaded);
    }
    return reloaded;
}

} // namespace Core
