#pragma once

#include <cstdint>
#include <utility>

#include <GL/glew.h>

#include "gl_error_handle.h"

namespace VkmGL {

/**
 * @brief RAII cubemap texture wrapper (per-face / per-mip render targets).
 *
 * Header-only so it needs no addition to the vkmGL source list. Owns a
 * GL_TEXTURE_CUBE_MAP: allocate all six faces (+ optional mip chain),
 * generate mips, bind to a sampler slot, or attach a face/mip to an FBO for
 * rendering. Non-copyable; movable (the moved-from object is left empty).
 */
class TextureCube {
    public:
        TextureCube() = default;
        ~TextureCube() { release(); }

        TextureCube(const TextureCube& other) = delete;
        TextureCube& operator=(const TextureCube& other) = delete;

        TextureCube(TextureCube && other) noexcept { *this = std::move(other); }
        TextureCube& operator=(TextureCube && other) noexcept {
            if (this != &other) {
                release();
                m_id   = other.m_id;
                m_size = other.m_size;
                m_mips = other.m_mips;
                other.m_id = 0;
            }
            return *this;
        }

    public:
        /**
         * @brief Allocate the cubemap (six faces, `mips` levels).
         * @param size           Per-face edge length in texels.
         * @param mips           Mip level count (1 = no chain).
         * @param internalFormat e.g. GL_RGB16F.
         * @param format/type    Pixel transfer format (data is null = storage only).
         * @param mipFiltering   Trilinear minification when true.
         */
        void create(int size, int mips, GLenum internalFormat,
                     GLenum format, GLenum type, bool mipFiltering) {
            release();
            m_size = size;
            m_mips = mips;

            VKM_GL_CHECK(glGenTextures(1, &m_id));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));
            for (int face = 0; face < 6; ++face) {
                VKM_GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalFormat,
                        size, size, 0, format, type, nullptr));
            }
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    mipFiltering ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            if (mips > 1) {
                VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mips - 1));
                VKM_GL_CHECK(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));  // allocate the chain
            }
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
        }

        void generateMipmaps() const {
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));
            VKM_GL_CHECK(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
        }

        void bindSlot(uint32_t slot) const {
            VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));
        }

        /// Attach one face/mip to the currently bound (or given) FBO.
        void attachFace(GLenum attachment, int face, int mip) const {
            VKM_GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_id, mip));
        }

        GLuint id()   const { return m_id; }
        int    size() const { return m_size; }
        int    mips() const { return m_mips; }
        bool   valid() const { return m_id != 0; }

    private:
        void release() noexcept {
            if (m_id) {
                VKM_GL_CHECK(glDeleteTextures(1, &m_id));
                m_id = 0;
            }
        }

    private:
        GLuint m_id   = 0;
        int    m_size = 0;
        int    m_mips = 1;
};

} // namespace VkmGL
