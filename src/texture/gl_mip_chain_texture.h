#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "gl_error_handle.h"
#include "gl_object.h"

namespace Vkm::GL {

/**
 * @brief RAII single-texture explicit mip chain, walked a level at a time.
 *
 * One GL texture with immutable storage for the whole chain. Every level
 * binds as an image for a compute pass to write (bindImage) and reads on its
 * own through a one-level texture view (bindLevel); the whole chain also binds
 * as one sampler for a reader that picks a level with textureLod (bindSlot).
 * The caller owns the format / filter / mip-count policy and passes it to
 * create(); this class owns the handles.
 *
 * Built for the per-level walk - write level N from level N-1 - that the
 * bloom, GTAO and reflection chains all do. A view of one level makes that walk
 * a read of a texture that does not contain the level being written, so it is
 * never a feedback loop and never needs the sampled range narrowed.
 */
class MipChainTexture : public GLObject {
    public:
        MipChainTexture() : GLObject(GL_TEXTURE_2D, GL_TEXTURE, 0) {}
        ~MipChainTexture() override { release(); }

        MipChainTexture(const MipChainTexture& other) = delete;
        MipChainTexture& operator=(const MipChainTexture& other) = delete;

        MipChainTexture(MipChainTexture && other) = delete;
        MipChainTexture& operator=(MipChainTexture && other) = delete;

        /**
         * @brief (Re)allocate the chain: `mips` levels, baseW x baseH at level 0
         *        and halving each level. Replaces any previous allocation.
         * @param baseW/baseH    Level-0 dimensions in texels.
         * @param mips           Mip level count (each level halves, min 1 texel).
         * @param internalFormat Sized format, e.g. GL_RGBA16F.
         * @param minFilter/magFilter  Sampling filters for the whole chain; a
         *        one-level view samples with magFilter both ways.
         */
        void create(int baseW, int baseH, int mips,
                    GLenum internalFormat, GLenum minFilter, GLenum magFilter) {
            release();
            m_baseW  = baseW;
            m_baseH  = baseH;
            m_mips   = mips;
            m_format = internalFormat;

            VKM_GL_CHECK(glGenTextures(1, &m_id));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
            VKM_GL_CHECK(glTexStorage2D(GL_TEXTURE_2D, mips, internalFormat, baseW, baseH));
            setSampling(minFilter, magFilter);

            m_levels.resize(static_cast<size_t>(mips));
            VKM_GL_CHECK(glGenTextures(mips, m_levels.data()));
            for (int mip = 0; mip < mips; ++mip) {
                const GLuint view = m_levels[static_cast<size_t>(mip)];
                VKM_GL_CHECK(glTextureView(view, GL_TEXTURE_2D, m_id, internalFormat,
                                           static_cast<GLuint>(mip), 1, 0, 1));
                VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, view));
                setSampling(magFilter, magFilter);
            }
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
        }

        bool isReady()  const { return m_id != 0; }
        int  mipCount() const { return m_mips; }

        int mipWidth (int mip) const { return std::max(m_baseW >> mip, 1); }
        int mipHeight(int mip) const { return std::max(m_baseH >> mip, 1); }

        /// Bind the whole chain for sampling (the shader selects a level via textureLod).
        void bindSlot(uint32_t slot) const {
            VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
        }

        /// Bind one level alone for sampling; the shader reads it as level 0.
        void bindLevel(int mip, uint32_t slot) const {
            VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_levels[static_cast<size_t>(mip)]));
        }

        /**
         * @brief Bind one level as an image, for a compute pass to write or read.
         *
         * No framebuffer is involved, which is the point: a chain walked in
         * compute pays for a dispatch per level, not a framebuffer bind and a
         * draw.
         *
         * @param mip    Level to bind.
         * @param unit   Image unit the shader names.
         * @param access GL_READ_ONLY, GL_WRITE_ONLY or GL_READ_WRITE.
         */
        void bindImage(int mip, uint32_t unit, GLenum access) const {
            VKM_GL_CHECK(glBindImageTexture(unit, m_id, mip, GL_FALSE, 0, access, m_format));
        }

    private:
        /// Wrap and filter for the texture bound to GL_TEXTURE_2D.
        static void setSampling(GLenum minFilter, GLenum magFilter) {
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
        }

        void release() noexcept {
            if (!m_levels.empty()) {
                VKM_GL_CHECK(glDeleteTextures(static_cast<GLsizei>(m_levels.size()), m_levels.data()));
                m_levels.clear();
            }
            if (m_id == 0) return;
            VKM_GL_CHECK(glDeleteTextures(1, &m_id));
            m_id = 0;
        }

    private:
        std::vector<GLuint> m_levels;  ///< One single-level view per mip.
        int    m_baseW  = 0;
        int    m_baseH  = 0;
        int    m_mips   = 1;
        GLenum m_format = GL_RGBA16F;
};

} // namespace Vkm::GL
