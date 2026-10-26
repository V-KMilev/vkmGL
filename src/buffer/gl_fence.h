#pragma once

#include <GL/glew.h>

namespace Vkm::GL {

/**
 * @brief A point in the command stream the CPU can ask the GPU about: whether everything issued
 *        before it has finished.
 *
 * Owns its GL sync object; holds none until place().
 */
class Fence {
    public:
        Fence() = default;
        ~Fence();

        Fence(const Fence& other) = delete;
        Fence& operator=(const Fence& other) = delete;

        Fence(Fence && other) noexcept;
        Fence& operator=(Fence && other) noexcept;

    public:
        /// Mark the commands issued so far, dropping any earlier mark.
        void place();

        /**
         * @brief Whether the marked commands have finished, without waiting; a fence that
         *        has is let go.
         *
         * @return True when they have, or none were marked.
         */
        bool passed();

        /// Wait until the marked commands have finished, then let the fence go.
        void wait();

        /// Whether it marks commands not yet known to have finished.
        bool pending() const { return m_sync != nullptr; }

    private:
        /// Delete the sync object, if any.
        void release() noexcept;

    private:
        GLsync m_sync = nullptr;
};

} // namespace Vkm::GL
