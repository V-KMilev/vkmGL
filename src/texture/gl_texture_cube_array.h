#pragma once

#include <algorithm>
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
 * you can address. Bind to a sampler slot; one layer/face/mip is drawn into through
 * FrameBuffer::attachLayer, at layer * 6 + face.
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

            VKM_GL_CHECK(glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_id));
            VKM_GL_CHECK(glTextureStorage3D(m_id, mips, internalFormat, size, size, capacity * 6));
            VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
            const GLint minFilter = mips > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
            VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, minFilter));
            VKM_GL_CHECK(glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        }

        void bindSlot(uint32_t slot) const {
            VKM_GL_CHECK(glBindTextureUnit(slot, m_id));
        }

        /**
         * @brief Copy cube @p from, every face of every level, over cube @p to.
         *
         * @param from The cube copied, below capacity().
         * @param to   The cube overwritten, below capacity().
         */
        void copyCube(int from, int to) const {
            for (int mip = 0; mip < m_mips; ++mip) {
                const int edge = std::max(m_size >> mip, 1);
                VKM_GL_CHECK(glCopyImageSubData(
                    m_id,
                    GL_TEXTURE_CUBE_MAP_ARRAY,
                    mip,
                    0,
                    0,
                    from * 6,
                    m_id,
                    GL_TEXTURE_CUBE_MAP_ARRAY,
                    mip,
                    0,
                    0,
                    to * 6,
                    edge,
                    edge,
                    6
                ));
            }
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
