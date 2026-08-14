#pragma once

#include "gl_object.h"
#include <cstdint>

namespace Core {

/**
* @brief Abstract base class for OpenGL buffer objects.
*
* Provides a common interface and resource management for GPU buffer objects.
* Not constructible directly, only via derived classes (e.g., VertexBuffer, ShaderStorageBuffer).
*/
class GLBuffer : public GLObject {
    public:
        GLBuffer() = delete;

        GLBuffer(const GLBuffer& other) = delete;
        GLBuffer& operator=(const GLBuffer& other) = delete;

        GLBuffer(GLBuffer && other) noexcept = default;
        GLBuffer& operator=(GLBuffer && other) noexcept;

    protected:
        GLBuffer(GLenum target, const void* data, uint32_t size, GLenum usage);
        ~GLBuffer() override;

    public:
        /**
        * @brief Binds the buffer to the specified target.
        * @param target OpenGL buffer target (default is GL_NONE, uses the internal target).
        */
        void bind(GLenum target = GL_NONE) const override;

        /**
        * @brief Unbinds the buffer from the specified target.
        * @param target OpenGL buffer target (default is GL_NONE, uses the internal target).
        */
        void unbind(GLenum target = GL_NONE) const override;

        /**
        * @brief Returns the buffer size in bytes.
        */
        uint32_t getSize() const;

        /**
        * @brief Updates a subset or the entirety of the buffer's data.
        */
        void update(const void* data, uint32_t size, uint32_t offset = 0);

        /**
        * @brief Reallocate the buffer's storage, discarding its contents.
        *
        * Orphaning: the GL name is unchanged, so vertex-array attribute
        * bindings that reference this buffer stay valid across a grow. Goes
        * through the wrapper rather than glBufferData directly so the tracked
        * size cannot drift from the storage it describes.
        *
        * @param size New size in bytes.
        * @param data Optional initial contents (null allocates only).
        */
        void allocate(uint32_t size, const void* data = nullptr);

        /**
        * @brief Maps the buffer into the client's address space.
        */
        void* map(GLenum access = GL_WRITE_ONLY);

        /**
        * @brief Maps a range of the buffer into the client's address space.
        */
        void* mapRange(uint32_t offset, uint32_t length, GLbitfield access);

        /**
        * @brief Unmaps the previously mapped buffer.
        */
        void unmap();

    protected:
        uint32_t m_size;
        GLenum m_usage;

    private:
        /// Delete the GL buffer and zero m_id. Idempotent — safe on a
        /// moved-from buffer or after a previous release().
        void release() noexcept;
};

};
