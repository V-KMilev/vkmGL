#pragma once

#include <cstdint>

#include "gl_fence.h"
#include "gl_object.h"

namespace Vkm::GL {

class GLBuffer;

/**
 * @brief A buffer the GPU copies into and the CPU reads once the copy has finished, without
 *        ever waiting for it.
 *
 * Mapped for reading for its whole life (persistent and coherent), so a read is a pointer, not a
 * map. capture() copies and places a fence; data() is the copy once arrived() says it has landed.
 * Several in turn let the CPU read a few frames behind the GPU.
 */
class ReadbackBuffer : public GLObject {
    public:
        ReadbackBuffer() = delete;
        ~ReadbackBuffer() override;

        ReadbackBuffer(const ReadbackBuffer& other) = delete;
        ReadbackBuffer& operator=(const ReadbackBuffer& other) = delete;

        ReadbackBuffer(ReadbackBuffer && other) noexcept;
        ReadbackBuffer& operator=(ReadbackBuffer && other) noexcept;

        /**
         * @brief Make and map @p bytes of storage.
         *
         * @param bytes Its size.
         * @throws std::runtime_error When the driver will not map it, having let it go.
         */
        explicit ReadbackBuffer(uint32_t bytes);

    public:
        /**
         * @brief Copy the first size() bytes of @p source, from @p offset, and mark when they land.
         *
         * @param source The buffer read from.
         * @param offset Where in it the copy begins.
         */
        void capture(const GLBuffer& source, uint32_t offset = 0);

        /**
         * @brief Whether the capture in flight has landed, asked without waiting.
         *
         * True once a capture: it is then no longer in flight.
         *
         * @return False while it is in flight, and with none in flight.
         */
        bool arrived();

        /// Whether a capture is in flight.
        bool pending() const { return m_fence.pending(); }

        /// Forget a capture in flight: it is never read.
        void drop();

        /// The copy, coherent with the GPU: read only once arrived() says it has landed.
        const void* data() const { return m_data; }

        uint32_t size() const { return m_bytes; }

    private:
        /// Unmap and delete the buffer and zero m_id.
        void release() noexcept;

    private:
        uint32_t    m_bytes = 0;
        const void* m_data  = nullptr;
        Fence       m_fence;
};

} // namespace Vkm::GL
