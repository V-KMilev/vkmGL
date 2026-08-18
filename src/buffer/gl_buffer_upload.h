#pragma once

#include <cstring>
#include <memory>

#include "gl_uniform_buffer.h"
#include "gl_shader_storage_buffer.h"

namespace VkmGL {

/**
 * @brief Memoized buffer upload: send @p data only when it differs from the
 * cached @p last.
 *
 * Creates the buffer on first use, otherwise re-uploads only when the POD
 * struct changed (memcmp) - so per-frame callers with static data cost no GPU
 * write. Works for any buffer wrapper with the (data, size, usage) constructor
 * and update(data, size) shape (UniformBuffer, ShaderStorageBuffer).
 *
 * @param buffer Buffer wrapper, created full-size on first call.
 * @param last   CPU mirror of the buffer's current GPU contents.
 * @param data   The struct to upload when changed.
 */
template <typename Buffer, typename T>
inline void uploadIfChanged(std::unique_ptr<Buffer>& buffer, T& last, const T& data) {
    const bool first = !buffer;
    if (!first && std::memcmp(&data, &last, sizeof(T)) == 0) return;
    if (first) buffer = std::make_unique<Buffer>(&data, sizeof(T), GL_DYNAMIC_DRAW);
    else       buffer->update(&data, sizeof(T));
    last = data;
}

/**
 * @brief Prefix variant of uploadIfChanged for header + count-guarded-array
 * buffers: compares and uploads only the first @p activeSize bytes.
 *
 * The buffer is still allocated at full size on first use so the prefix can
 * grow without reallocation, and @p last is only overwritten inside the
 * prefix - bytes past it therefore always mirror what the GPU holds, keeping
 * the comparison sound when the active count shrinks and later regrows.
 *
 * @param buffer     Buffer wrapper, created full-size on first call.
 * @param last       CPU mirror of the buffer's current GPU contents.
 * @param data       This frame's full struct (only the prefix is considered).
 * @param activeSize Bytes actually read by the shaders this frame.
 */
template <typename Buffer, typename T>
inline void uploadPrefixIfChanged(std::unique_ptr<Buffer>& buffer, T& last,
                                  const T& data, size_t activeSize) {
    const bool first = !buffer;
    if (!first && std::memcmp(&data, &last, activeSize) == 0) return;
    if (first) buffer = std::make_unique<Buffer>(&data, sizeof(T), GL_DYNAMIC_DRAW);
    else       buffer->update(&data, static_cast<uint32_t>(activeSize));
    std::memcpy(&last, &data, activeSize);
}

} // namespace VkmGL
