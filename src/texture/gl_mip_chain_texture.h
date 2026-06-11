#pragma once

#include <algorithm>
#include <cstdint>
#include <utility>

#include <GL/glew.h>

#include "gl_error_handle.h"

namespace Core {

/**
 * @brief RAII single-texture explicit mip-chain render target.
 *
 * Header-only so it needs no addition to the vkmGL source list. One GL texture
 * with an explicit per-level mip chain plus a reusable FBO: each level is
 * independently render-targetable (attachMip), and the whole chain binds as one
 * sampler (the shader selects a level via textureLod). The caller owns the
 * format / filter / mip-count policy and passes it to create(); this class owns
 * the handles and the bind / attach / query ops. Non-copyable; movable.
 */
class MipChainTexture {
    public:
        MipChainTexture() = default;
        ~MipChainTexture() { release(); }

        MipChainTexture(const MipChainTexture& other) = delete;
        MipChainTexture& operator=(const MipChainTexture& other) = delete;

        MipChainTexture(MipChainTexture && other) noexcept { *this = std::move(other); }
        MipChainTexture& operator=(MipChainTexture && other) noexcept {
            if (this != &other) {
                release();
                m_tex   = other.m_tex;
                m_fbo   = other.m_fbo;
                m_baseW = other.m_baseW;
                m_baseH = other.m_baseH;
                m_mips  = other.m_mips;
                other.m_tex = 0;
                other.m_fbo = 0;
            }
            return *this;
        }

        /**
         * @brief (Re)allocate the chain: a `mips`-level texture (baseW x baseH at
         *        level 0, halving each level) + a reusable FBO. Replaces any
         *        previous allocation.
         * @param baseW/baseH    Level-0 dimensions in texels.
         * @param mips           Mip level count (each level halves, min 1 texel).
         * @param internalFormat e.g. GL_RGBA16F.
         * @param format/type    Pixel transfer format (data is null = storage only).
         * @param minFilter/magFilter  Sampling filters.
         */
        void create(int baseW, int baseH, int mips,
                    GLenum internalFormat, GLenum format, GLenum type,
                    GLenum minFilter, GLenum magFilter) {
            release();
            m_baseW = baseW;
            m_baseH = baseH;
            m_mips  = mips;

            VKM_GL_CHECK(glGenTextures(1, &m_tex));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_tex));
            for (int mip = 0; mip < mips; ++mip) {
                VKM_GL_CHECK(glTexImage2D(GL_TEXTURE_2D, mip, internalFormat,
                    mipWidth(mip), mipHeight(mip), 0, format, type, nullptr));
            }
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mips - 1));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

            VKM_GL_CHECK(glGenFramebuffers(1, &m_fbo));
        }

        bool   isReady()  const { return m_tex != 0; }
        int    mipCount() const { return m_mips; }
        GLuint id()       const { return m_tex; }

        int mipWidth (int mip) const { return std::max(m_baseW >> mip, 1); }
        int mipHeight(int mip) const { return std::max(m_baseH >> mip, 1); }

        /// Bind the chain texture for sampling (shader selects a level via textureLod).
        void bindSlot(uint32_t slot) const {
            VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_tex));
        }

        /// Bind / unbind the chain's framebuffer for the per-mip loop.
        void bindFbo()   const { VKM_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo)); }
        void unbindFbo() const { VKM_GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0)); }

        /// Point COLOR_ATTACHMENT0 at one chain mip and size the viewport to it.
        void attachMip(int mip) const {
            VKM_GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, mip));
            VKM_GL_CHECK(glViewport(0, 0, mipWidth(mip), mipHeight(mip)));
        }

    private:
        void release() noexcept {
            if (m_fbo) { VKM_GL_CHECK(glDeleteFramebuffers(1, &m_fbo)); }
            if (m_tex) { VKM_GL_CHECK(glDeleteTextures(1, &m_tex)); }
            m_fbo = 0;
            m_tex = 0;
        }

        GLuint m_tex   = 0;
        GLuint m_fbo   = 0;
        int    m_baseW = 0;
        int    m_baseH = 0;
        int    m_mips  = 1;
};

} // namespace Core
