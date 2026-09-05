#pragma once

#include <algorithm>
#include <cstdint>
#include <utility>

#include <GL/glew.h>

#include "gl_error_handle.h"
#include "gl_frame_buffer.h"
#include "gl_object.h"

namespace Vkm::GL {

/**
 * @brief RAII single-texture explicit mip-chain render target.
 *
 * One GL texture with an explicit per-level mip chain plus a reusable FBO: each
 * level is independently render-targetable (attachMip), and the whole chain
 * binds as one sampler (the shader selects a level via textureLod). The caller
 * owns the format / filter / mip-count policy and passes it to create(); this
 * class owns the handles and the bind / attach / query ops.
 *
 * A GLObject for the texture, so its ownership rules are the base's (see
 * TextureCube), and a FrameBuffer member for the FBO rather than a raw name -
 * this module already has a type that owns one.
 */
class MipChainTexture : public GLObject {
    public:
        MipChainTexture() : GLObject(GL_TEXTURE_2D, GL_TEXTURE, 0) {}
        ~MipChainTexture() override { release(); }

        MipChainTexture(const MipChainTexture& other) = delete;
        MipChainTexture& operator=(const MipChainTexture& other) = delete;

        MipChainTexture(MipChainTexture && other) noexcept
            : GLObject(std::move(other))
            , m_fbo(std::move(other.m_fbo))
            , m_baseW(other.m_baseW), m_baseH(other.m_baseH), m_mips(other.m_mips) {}

        MipChainTexture& operator=(MipChainTexture && other) noexcept {
            if (this != &other) {
                release();
                GLObject::operator=(std::move(other));
                m_fbo   = std::move(other.m_fbo);
                m_baseW = other.m_baseW;
                m_baseH = other.m_baseH;
                m_mips  = other.m_mips;
            }
            return *this;
        }

        /**
         * @brief (Re)allocate the chain: a `mips`-level texture, baseW x baseH at
         *        level 0 and halving each level. Replaces any previous
         *        allocation; the FBO is the member's and outlives them all.
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

            VKM_GL_CHECK(glGenTextures(1, &m_id));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
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
        }

        bool isReady()  const { return m_id != 0; }
        int  mipCount() const { return m_mips; }

        int mipWidth (int mip) const { return std::max(m_baseW >> mip, 1); }
        int mipHeight(int mip) const { return std::max(m_baseH >> mip, 1); }

        /// Bind the chain texture for sampling (shader selects a level via textureLod).
        void bindSlot(uint32_t slot) const {
            VKM_GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
        }

        /// Bind / unbind the chain's framebuffer for the per-mip loop.
        void bindFbo()   const { m_fbo.bind(); }
        void unbindFbo() const { FrameBuffer::bindDefault(); }

        /**
         * @brief Restrict which levels sampling may read.
         *
         * Rendering into one level of a texture while sampling another is only
         * defined if the sampled range excludes the attached level. Callers
         * walking the chain (write mip N, read mip N-1) set this to N-1 so the
         * two never overlap.
         *
         * @param maxLevel Highest level sampling may read.
         */
        void restrictSampling(int maxLevel) const {
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxLevel));
        }

        /// Undo restrictSampling: the whole chain is readable again.
        void allowAllSampling() const {
            VKM_GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
            VKM_GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_mips - 1));
        }

        /// Point COLOR_ATTACHMENT0 at one chain mip and size the viewport to it.
        void attachMip(int mip) const {
            VKM_GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_id, mip));
            VKM_GL_CHECK(glViewport(0, 0, mipWidth(mip), mipHeight(mip)));
        }

    private:
        void release() noexcept {
            if (m_id == 0) return;
            VKM_GL_CHECK(glDeleteTextures(1, &m_id));
            m_id = 0;
        }

    private:
        FrameBuffer m_fbo;      ///< Reused across the per-mip loop; owns its own name.
        int         m_baseW = 0;
        int         m_baseH = 0;
        int         m_mips  = 1;
};

} // namespace Vkm::GL
