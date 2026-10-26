#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "gl_buffer.h"  // BufferSpan
#include "gl_fence.h"

namespace Vkm::GL {

/**
 * @brief Per-frame data written straight into memory the GPU reads: one persistently mapped
 *        buffer, cut into FRAMES regions taken in turn.
 *
 * A region opens on the first write() after the last endFrame(), waiting first until the GPU
 * has finished the frame that last used it, and endFrame() fences it. So the CPU never writes
 * what the GPU still reads, never waits on the frame it is building, and the driver never has
 * to copy or rename a buffer to let a draw keep its old contents: what glBufferSubData into a
 * buffer in flight costs. Writes outside any frame - an offline bake, an editor preview - join
 * the region the next endFrame() fences.
 *
 * Spans are aligned for any use: a storage block's range, an instanced vertex stream, indirect
 * commands. A region too small for a frame doubles, into a new buffer; the old one is deleted
 * once every frame that could read it has finished.
 */
class StreamRing {
    public:
        static constexpr uint32_t FRAMES = 3;

        /**
         * @brief An empty ring; nothing is allocated before the first write().
         *
         * @param initialBytes What a region holds at first.
         */
        explicit StreamRing(uint32_t initialBytes = 1u << 20);
        ~StreamRing();

        StreamRing(const StreamRing& other) = delete;
        StreamRing& operator=(const StreamRing& other) = delete;

        StreamRing(StreamRing && other) = delete;
        StreamRing& operator=(StreamRing && other) = delete;

    public:
        /**
         * @brief Copy @p bytes of @p data into the open region.
         *
         * @param data  The bytes.
         * @param bytes How many; 0 writes nothing and returns an empty span.
         * @return Where they went.
         */
        BufferSpan write(const void* data, uint32_t bytes);

        /// Fence the open region: the GPU commands issued so far are the last to read it.
        void endFrame();

        /// Bytes a region holds now.
        uint32_t regionBytes() const { return m_regionBytes; }

    private:
        struct Retired {
            GLuint   buffer;
            uint32_t frame;   ///< The frame it was replaced in; deleted FRAMES frames on.
        };

    private:
        /// Open the next region, waiting for the GPU to finish the frame that last used it.
        void open();

        /// Make the buffer, or move to a new one whose regions hold at least @p bytes and twice the old.
        void grow(uint32_t bytes);

        void release() noexcept;

    private:
        GLuint   m_buffer      = 0;
        uint8_t* m_mapped      = nullptr;
        uint32_t m_regionBytes = 0;
        uint32_t m_region      = 0;      ///< The region open, or last open.
        uint32_t m_cursor      = 0;      ///< Bytes of it taken.
        uint32_t m_alignment   = 256;
        uint32_t m_frame       = 0;      ///< Frames ended.
        bool     m_open        = false;

        std::array<Fence, FRAMES> m_fences;
        std::vector<Retired>      m_retired;
};

/**
 * @brief Per-frame data the GPU reads many times a frame: written through a StreamRing, then
 *        copied once into a buffer in the GPU's own memory.
 *
 * A ring's memory may sit on the CPU's side of the bus, where every pass reading it - a
 * prepass, a forward pass, each shadow tile - fetches it across, and those fetches take
 * memory bandwidth from the CPU. One copy a frame, ordered after the last frame's reads, keeps
 * the ring's no-wait write and reads at video memory speed.
 */
class DeviceStream {
    public:
        DeviceStream() = default;
        ~DeviceStream();

        DeviceStream(const DeviceStream& other) = delete;
        DeviceStream& operator=(const DeviceStream& other) = delete;

        DeviceStream(DeviceStream && other) = delete;
        DeviceStream& operator=(DeviceStream && other) = delete;

    public:
        /**
         * @brief Write @p bytes of @p data through @p ring and copy them into the device buffer.
         *
         * @param ring  The frame's ring.
         * @param data  The bytes.
         * @param bytes How many; 0 copies nothing, returns an empty span and leaves span() empty.
         * @return The device buffer's range holding them, from offset 0.
         */
        BufferSpan upload(StreamRing& ring, const void* data, uint32_t bytes);

        /**
         * @brief Write @p bytes of @p data through @p ring over the device buffer from @p offset,
         *        leaving the rest as the last upload() left it.
         *
         * @param ring   The frame's ring.
         * @param offset Where in the device buffer they go; with @p bytes, within span().
         * @param data   The bytes.
         * @param bytes  How many.
         */
        void patch(StreamRing& ring, uint32_t offset, const void* data, uint32_t bytes);

        /**
         * @brief Make span() @p bytes long, keeping what it holds up to the shorter of the two.
         *
         * Bytes past the old length are undefined until patched. Growing past the buffer moves it
         * into a larger one on the device.
         *
         * @param bytes The new length.
         */
        void resize(uint32_t bytes);

        /// The device buffer's range the last upload() filled; empty before the first.
        BufferSpan span() const { return BufferSpan{m_buffer, 0, m_bytes}; }

    private:
        GLuint   m_buffer   = 0;
        uint32_t m_capacity = 0;
        uint32_t m_bytes    = 0;  ///< What the last upload() filled.
};

} // namespace Vkm::GL
