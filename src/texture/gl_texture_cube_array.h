#pragma once

#include <cstdint>
#include <utility>

#include <GL/glew.h>

#include "gl_error_handle.h"
#include "gl_object.h"

namespace Vkm::GL {

/**
 * @brief RAII cubemap-array texture wrapper (per-layer / per-face / per-mip targets).
 *
 * Owns a GL_TEXTURE_CUBE_MAP_ARRAY allocated with immutable storage: `capacity` cubes,
 * each six faces, with an optional mip chain. A single sampler binds the whole
 * array regardless of layer count, and the shader selects a cube via its layer
 * index - so the layer count, not the texture-unit budget, bounds how many cubes
 * you can address. Bind to a sampler slot, or attach one layer/face/mip to an FBO
 * for rendering.
 *
 * A GLObject, so its ownership rules are the base's; see TextureCube.
 */
class TextureCubeArray : public GLObject {
    public:
        TextureCubeArray() : GLObject(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE, 0) {}
        ~TextureCubeArray() override { release(); }

        TextureCubeArray(const TextureCubeArray& other) = delete;
        TextureCubeArray& operator=(const TextureCubeArray& other) = delete;

        TextureCubeArray(TextureCubeArray && other) noexcept
            : GLObject(std::move(other))
            , m_size(other.m_size), m_mips(other.m_mips), m_capacity(other.m_capacity) {}

        TextureCubeArray& operator=(TextureCubeArray && other) noexcept {
            if (this != &other) {
                release();
                GLObject::operator=(std::move(other));
                m_size     = other.m_size;
                m_mips     = other.m_mips;
                m_capacity = other.m_capacity;
            }
            return *this;
        }

    public:
        /**
         * @brief Allocate the array (`capacity` cubes, `mips` levels each) with immutable storage.
         * @param size           Per-face edge length in texels.
         * @param mips           Mip level count (1 = no chain).
         * @param capacity       Number of cubes (the array holds capacity * 6 layer-faces).
         * @param internalFormat Sized internal format, e.g. GL_RGBA16F (the guaranteed
         *                       colour-renderable HDR format; RGB16F render support is optional).
         */
        void create(int size, int mips, int capacity, GLenum internalFormat) {
            release();
            m_size     = size;
            m_mips     = mips;
            m_capacity = capacity;

            VKM_GL_CHECK(glGenTextures(1, &m_id));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_id));
            VKM_GL_CHECK(glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mips, internalFormat, size, size, capacity * 6));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER,
                mips > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, mips - 1));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0));
        }

        void bindSlot(uint32_t slot) const {
            VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_id));
        }

        /// Attach one layer/face/mip to the currently bound FBO. The flat index is
        /// `layer * 6 + face`, matching the cube-array layer-face layout.
        void attachFace(GLenum attachment, int layer, int face, int mip) const {
            VKM_GL_CHECK(glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, m_id, mip, layer * 6 + face));
        }

        int size()     const { return m_size; }
        int mips()     const { return m_mips; }
        int capacity() const { return m_capacity; }

    private:
        void release() noexcept {
            if (m_id == 0) return;
            VKM_GL_CHECK(glDeleteTextures(1, &m_id));
            m_id = 0;
        }

    private:
        int m_size     = 0;
        int m_mips     = 1;
        int m_capacity = 0;
};

} // namespace Vkm::GL
