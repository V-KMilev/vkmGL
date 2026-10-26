#include "gl_stream_ring.h"

#include <algorithm>
#include <cstring>

#include "gl_error_handle.h"
#include "l_assert.h"

namespace Vkm::GL {

namespace {

constexpr GLbitfield MAPPING = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

/// @p bytes rounded up to a multiple of @p alignment, a power of two.
uint32_t alignUp(uint32_t bytes, uint32_t alignment) {
    return (bytes + alignment - 1) & ~(alignment - 1);
}

/**
 * @brief The alignment every span keeps: the larger of a storage and a uniform block's.
 *
 * @return At least 16 bytes, a power of two.
 */
uint32_t spanAlignment() {
    GLint storage = 16;
    GLint uniform = 16;
    VKM_GL_CHECK(glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &storage));
    VKM_GL_CHECK(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform));
    uint32_t alignment = 16;
    while (alignment < static_cast<uint32_t>(std::max(storage, uniform))) alignment <<= 1;
    return alignment;
}

} // namespace

StreamRing::StreamRing(uint32_t initialBytes)
    : m_regionBytes(initialBytes) {}

StreamRing::~StreamRing() {
    release();
}

void StreamRing::release() noexcept {
    for (Fence& fence : m_fences) fence = Fence{};
    for (const Retired& retired : m_retired) VKM_GL_CHECK(glDeleteBuffers(1, &retired.buffer));
    m_retired.clear();
    if (m_buffer == 0) return;
    VKM_GL_CHECK(glUnmapNamedBuffer(m_buffer));
    VKM_GL_CHECK(glDeleteBuffers(1, &m_buffer));
    m_buffer = 0;
    m_mapped = nullptr;
}

BufferSpan StreamRing::write(const void* data, uint32_t bytes) {
    if (bytes == 0) return {};
    if (!m_open) open();

    uint32_t start = alignUp(m_cursor, m_alignment);
    if (m_buffer == 0 || start + bytes > m_regionBytes) {
        grow(bytes);
        start = 0;
    }
    const uint32_t offset = m_region * m_regionBytes + start;
    std::memcpy(m_mapped + offset, data, bytes);
    m_cursor = start + bytes;
    return BufferSpan{m_buffer, offset, bytes};
}

void StreamRing::endFrame() {
    if (!m_open) return;
    m_fences[m_region].place();
    m_open = false;
    ++m_frame;
}

void StreamRing::open() {
    m_region = (m_region + 1) % FRAMES;
    m_fences[m_region].wait();
    // The frame that last read a retired buffer is now done when FRAMES frames have ended since.
    const auto done = [this](const Retired& retired) { return retired.frame + FRAMES <= m_frame; };
    for (const Retired& retired : m_retired) {
        if (done(retired)) VKM_GL_CHECK(glDeleteBuffers(1, &retired.buffer));
    }
    m_retired.erase(std::remove_if(m_retired.begin(), m_retired.end(), done), m_retired.end());

    m_cursor = 0;
    m_open   = true;
}

void StreamRing::grow(uint32_t bytes) {
    if (m_buffer == 0) {
        m_alignment = spanAlignment();
    } else {
        // Earlier spans this frame still read the old buffer; it goes once they cannot.
        VKM_GL_CHECK(glUnmapNamedBuffer(m_buffer));
        m_retired.push_back(Retired{m_buffer, m_frame});
        m_regionBytes *= 2;
    }
    m_regionBytes = alignUp(std::max(m_regionBytes, bytes), m_alignment);

    VKM_GL_CHECK(glCreateBuffers(1, &m_buffer));
    VKM_ASSERT(m_buffer != 0);
    const GLsizeiptr size = static_cast<GLsizeiptr>(m_regionBytes) * FRAMES;
    VKM_GL_CHECK(glNamedBufferStorage(m_buffer, size, nullptr, MAPPING));
    void* mapped = nullptr;
    VKM_GL_CHECK(mapped = glMapNamedBufferRange(m_buffer, 0, size, MAPPING));
    m_mapped = static_cast<uint8_t*>(mapped);
    VKM_ASSERT(m_mapped != nullptr);
    m_cursor = 0;
}

DeviceStream::~DeviceStream() {
    if (m_buffer != 0) VKM_GL_CHECK(glDeleteBuffers(1, &m_buffer));
}

BufferSpan DeviceStream::upload(StreamRing& ring, const void* data, uint32_t bytes) {
    const BufferSpan staged = ring.write(data, bytes);
    if (!staged) {
        m_bytes = 0;
        return {};
    }
    if (bytes > m_capacity) {
        // Half again, so a scene that grows a little each frame reallocates rarely.
        if (m_buffer != 0) VKM_GL_CHECK(glDeleteBuffers(1, &m_buffer));
        m_capacity = bytes + bytes / 2;
        VKM_GL_CHECK(glCreateBuffers(1, &m_buffer));
        VKM_GL_CHECK(glNamedBufferStorage(m_buffer, m_capacity, nullptr, 0));
    }
    VKM_GL_CHECK(glCopyNamedBufferSubData(staged.buffer, m_buffer, staged.offset, 0, bytes));
    m_bytes = bytes;
    return span();
}

void DeviceStream::resize(uint32_t bytes) {
    if (bytes > m_capacity) {
        // Half again, as upload() grows.
        const uint32_t capacity = bytes + bytes / 2;
        GLuint grown = 0;
        VKM_GL_CHECK(glCreateBuffers(1, &grown));
        VKM_GL_CHECK(glNamedBufferStorage(grown, capacity, nullptr, 0));
        if (m_bytes > 0) VKM_GL_CHECK(glCopyNamedBufferSubData(m_buffer, grown, 0, 0, m_bytes));
        if (m_buffer != 0) VKM_GL_CHECK(glDeleteBuffers(1, &m_buffer));
        m_buffer   = grown;
        m_capacity = capacity;
    }
    m_bytes = bytes;
}

void DeviceStream::patch(StreamRing& ring, uint32_t offset, const void* data, uint32_t bytes) {
    VKM_ASSERT(offset + bytes <= m_bytes);
    const BufferSpan staged = ring.write(data, bytes);
    if (!staged) return;
    VKM_GL_CHECK(glCopyNamedBufferSubData(staged.buffer, m_buffer, staged.offset, offset, bytes));
}

} // namespace Vkm::GL
