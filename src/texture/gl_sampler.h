#pragma once

#include "gl_object.h"
#include "gl_texture.h"

namespace Core {

/**
 * @brief Sampler object decoupling filter/wrap state from texture storage.
 *
 * Texture2D historically bakes wrap/filter into the texture object. A Sampler
 * holds that state separately and is bound to a texture unit independently
 * (glBindSampler), letting the same texture be sampled with different filters.
 * Mirrors the Vulkan VkSampler model.
 */
class Sampler : public GLObject {
    public:
        struct Params {
            TextureWrap wrapS = TextureWrap::Repeat;
            TextureWrap wrapT = TextureWrap::Repeat;
            TextureWrap wrapR = TextureWrap::Repeat;
            TextureMinFilter minFilter = TextureMinFilter::LinearMipmapLinear;
            TextureMagFilter magFilter = TextureMagFilter::Linear;
        };

        Sampler();
        explicit Sampler(const Params& params);
        ~Sampler() override;

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;

        Sampler(Sampler&&) noexcept = default;
        Sampler& operator=(Sampler&&) noexcept;

        /** @brief No-op for interface compat; use bindSlot(). */
        void bind(GLenum target = GL_NONE) const override;
        /** @brief No-op for interface compat; use unbindSlot(). */
        void unbind(GLenum target = GL_NONE) const override;

        /** @brief Bind this sampler to a specific texture unit. */
        void bindSlot(uint32_t slot) const;
        /** @brief Unbind any sampler from the given texture unit. */
        static void unbindSlot(uint32_t slot);

        /** @brief Update wrap/filter parameters on the live GL object. */
        void setParams(const Params& params);
        const Params& getParams() const { return m_params; }

    private:
        void applyParams();

        Params m_params;
};

} // namespace Core
