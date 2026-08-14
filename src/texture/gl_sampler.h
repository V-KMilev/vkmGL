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

        Sampler(const Sampler& other) = delete;
        Sampler& operator=(const Sampler& other) = delete;

        Sampler(Sampler && other) noexcept = default;
        Sampler& operator=(Sampler && other) noexcept;

    public:
        /**
         * @brief Bind this sampler to a texture unit.
         *
         * The sampler's wrap/filter state then overrides whatever the texture
         * bound to the same unit carries, which is the point of the type.
         *
         * @param slot Texture unit index.
         */
        void bindSlot(uint32_t slot) const;

        /**
         * @brief Unbind any sampler from a texture unit.
         *
         * Static because clearing a unit does not need an instance - the unit
         * reverts to sampling with the texture's own parameters.
         *
         * @param slot Texture unit index.
         */
        static void unbindSlot(uint32_t slot);

        /**
         * @brief Replace the wrap/filter parameters on the live GL object.
         *
         * @param params New parameter set, applied immediately.
         */
        void setParams(const Params& params);

        /**
         * @brief The parameters this sampler currently applies.
         *
         * @return Reference to the stored parameter set.
         */
        const Params& getParams() const { return m_params; }

    private:
        void applyParams();

    private:
        Params m_params;
};

} // namespace Core
