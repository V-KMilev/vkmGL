#pragma once

#include "gl_object.h"
#include <unordered_map>
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

        GLBuffer(GLBuffer && other) = delete;
        GLBuffer& operator=(GLBuffer && other) = delete;

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
        * @return Size of buffer.
        */
        uint32_t getSize() const;

        /**
        * @brief Updates a subset or the entirety of the buffer's data.
        * @param data   Pointer to the new data.
        * @param size   Size of the new data in bytes.
        * @param offset Offset in bytes into the buffer to update.
        */
        void update(const void* data, uint32_t size, uint32_t offset = 0);

        /**
        * @brief Maps the buffer into the client's address space.
        * @param access OpenGL access flag (e.g., GL_WRITE_ONLY).
        * @return Pointer to the mapped data.
        */
        void* map(GLenum access = GL_WRITE_ONLY);

        /**
        * @brief Maps a range of the buffer into the client's address space.
        * @param offset Offset in bytes to start mapping.
        * @param length Length in bytes to map.
        * @param access OpenGL access flags.
        * @return Pointer to the mapped data.
        */
        void* mapRange(uint32_t offset, uint32_t length, GLbitfield access);

        /**
        * @brief Unmaps the previously mapped buffer.
        */
        void unmap();

    protected:
        uint32_t m_size;
        GLenum m_usage;

        // Static state cache to track currently bound buffers per target
        static std::unordered_map<GLenum, uint32_t> s_boundBuffers;
};

};
