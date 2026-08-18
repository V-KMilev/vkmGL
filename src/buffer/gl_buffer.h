#pragma once

#include "gl_object.h"
#include <cstdint>

namespace Vkm::GL {

class GLBuffer;

/**
 * @brief A buffer mapped into client address space, unmapped on destruction.
 *
 * Obtained from GLBuffer::map() / mapRange(), never constructed directly. Move
 * transfers the mapping, so a moved-from object unmaps nothing.
 */
class MappedBuffer {
    public:
        MappedBuffer() = delete;
        ~MappedBuffer();

        MappedBuffer(const MappedBuffer& other) = delete;
        MappedBuffer& operator=(const MappedBuffer& other) = delete;

        MappedBuffer(MappedBuffer && other) noexcept;
        MappedBuffer& operator=(MappedBuffer && other) noexcept;

    public:
        /**
         * @brief The mapped memory.
         *
         * @return Pointer to the mapping, or null if the map failed.
         */
        void* data() const { return m_data; }

        /**
         * @brief Whether the mapping succeeded.
         *
         * @return True when data() is safe to write through.
         */
        bool isValid() const { return m_data != nullptr; }

    private:
        friend class GLBuffer;

        MappedBuffer(const GLBuffer& buffer, void* data);

    private:
        const GLBuffer* m_buffer;
        void*           m_data;
};

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
        void bind(GLenum target = GL_NONE) const;

        /**
         * @brief Unbinds the buffer from the specified target.
         * @param target OpenGL buffer target (default is GL_NONE, uses the internal target).
         */
        void unbind(GLenum target = GL_NONE) const;

        /**
         * @brief Returns the buffer size in bytes.
         *
         * @return Size of the current storage allocation.
         */
        uint32_t getSize() const;

        /**
         * @brief The target this buffer was created against.
         *
         * @return The GL_*_BUFFER constant bind() uses when given no override.
         */
        GLenum getTarget() const;

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
         * @brief Bind the whole buffer to an indexed binding point.
         *
         * Uses the buffer's own target, so a UniformBuffer binds as a UBO and a
         * ShaderStorageBuffer as an SSBO with no per-subclass override. Only the
         * indexed targets accept this (uniform, shader-storage, atomic-counter,
         * transform-feedback); calling it on a vertex or index buffer is a GL
         * error, which VKM_GL_CHECK reports in debug builds.
         *
         * @param bindingPoint Index of the binding point the shader declares.
         */
        void bindBase(uint32_t bindingPoint) const;

        /**
         * @brief Bind a sub-range of the buffer to an indexed binding point.
         *
         * Same target rule and same restriction as bindBase(); the shader sees
         * only the requested window.
         *
         * @param bindingPoint Index of the binding point the shader declares.
         * @param offset       Byte offset the range starts at.
         * @param size         Length of the range in bytes.
         */
        void bindRange(uint32_t bindingPoint, uint32_t offset, uint32_t size) const;

        /**
         * @brief Map the whole buffer into client address space.
         *
         * The mapping unmaps itself when the returned object goes out of
         * scope, which is why nothing here hands back a bare pointer: an
         * unmatched unmap leaves the buffer unusable by the GPU, and there is
         * no way to notice from the call site.
         *
         * @param access GL_READ_ONLY / GL_WRITE_ONLY / GL_READ_WRITE.
         * @return The mapping; check isValid() before writing through it.
         */
        [[nodiscard]] MappedBuffer map(GLenum access = GL_WRITE_ONLY);

        /**
         * @brief Map part of the buffer into client address space.
         *
         * Same scoped-unmap contract as map().
         *
         * @param offset Byte offset the mapping starts at.
         * @param length Length of the mapping in bytes.
         * @param access Bitfield of GL_MAP_* flags.
         * @return The mapping; check isValid() before writing through it.
         */
        [[nodiscard]] MappedBuffer mapRange(uint32_t offset, uint32_t length, GLbitfield access);

    private:
        /**
         * @brief Delete the GL buffer and zero m_id.
         *
         * Idempotent, so it is safe on a moved-from buffer or after a previous release().
         */
        void release() noexcept;

    protected:
        uint32_t m_size;
        GLenum   m_usage;
};

} // namespace Vkm::GL
