#include "RuntimeTextures.h"

#include "Render/RenderBase.h"

namespace DAVA
{
namespace
{

const FastName DYNAMIC_TEXTURE_NAMES[RuntimeTextures::DYNAMIC_TEXTURES_COUNT] =
{
    FastName("unknownTexture"),
    FastName("dynamicReflection"),
    FastName("dynamicRefraction")
};

}

RuntimeTextures::eDynamicTextureSemantic RuntimeTextures::GetDynamicTextureSemanticByName(const FastName& name)
{
    for (int32 k = 0; k < DYNAMIC_TEXTURES_COUNT; ++k)
        if (name == DYNAMIC_TEXTURE_NAMES[k])return (eDynamicTextureSemantic)k;
    return TEXTURE_STATIC;
}

rhi::HTexture RuntimeTextures::GetDynamicTexture(eDynamicTextureSemantic semantic)
{
    DVASSERT(semantic != TEXTURE_STATIC);
    DVASSERT(semantic < DYNAMIC_TEXTURES_COUNT);
    if (!dynamicTextures[semantic].IsValid())
        InitDynamicTexture(semantic);

    return dynamicTextures[semantic];
}

rhi::HTexture RuntimeTextures::GetPinkTexture(rhi::TextureType type)
{
    int32 textureIndex = type - 1;
    if (!pinkTexture[textureIndex])
        pinkTexture[textureIndex] = Texture::CreatePink(type);

    return pinkTexture[textureIndex]->handle;
}

rhi::SamplerState::Descriptor::Sampler RuntimeTextures::GetPinkTextureSamplerState(rhi::TextureType type)
{
    int32 textureIndex = type - 1;
    if (!pinkTexture[textureIndex])
        pinkTexture[textureIndex] = Texture::CreatePink(type);

    return pinkTexture[textureIndex]->samplerState;
}

void RuntimeTextures::ClearRuntimeTextures()
{
    for (int32 i = 0; i < DYNAMIC_TEXTURES_COUNT; i++)
    {
        if (dynamicTextures[i].IsValid())
        {
            rhi::DeleteTexture(dynamicTextures[i]);
            dynamicTextures[i] = rhi::HTexture();
        }
    }

    SafeRelease(pinkTexture[0]);
    SafeRelease(pinkTexture[1]);
}

void RuntimeTextures::InitDynamicTexture(eDynamicTextureSemantic semantic)
{
    DVASSERT(!dynamicTextures[semantic].IsValid());
    rhi::Texture::Descriptor descriptor;
    int32 size;
    switch (semantic)
    {    
    case DAVA::RuntimeTextures::TEXTURE_DYNAMIC_REFLECTION:
        descriptor.width = REFLECTION_TEX_SIZE;
        descriptor.height = REFLECTION_TEX_SIZE;
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_R5G6B5;                
        dynamicTextures[semantic] = rhi::CreateTexture(descriptor);
        break;
    case DAVA::RuntimeTextures::TEXTURE_DYNAMIC_REFRACTION:
        descriptor.width = REFRACTION_TEX_SIZE;
        descriptor.height = REFRACTION_TEX_SIZE;
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_R5G6B5;
        dynamicTextures[semantic] = rhi::CreateTexture(descriptor);
        break;

    case DAVA::RuntimeTextures::TEXTURE_DYNAMIC_RR_DEPTHBUFFER:
        size = Max(REFLECTION_TEX_SIZE, REFRACTION_TEX_SIZE);
        descriptor.width = size;
        descriptor.height = size;
        descriptor.autoGenMipmaps = false;        
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_D24S8;
        dynamicTextures[semantic] = rhi::CreateTexture(descriptor);
        break;
    
    default:
        DVASSERT_MSG(false, "Trying to init unknown texture as dynamic");
        break;
    }
}

rhi::SamplerState::Descriptor::Sampler RuntimeTextures::GetDynamicTextureSamplerState(eDynamicTextureSemantic semantic)
{
    rhi::SamplerState::Descriptor::Sampler sampler;
    sampler.addrU = rhi::TEXADDR_MIRROR;
    sampler.addrV = rhi::TEXADDR_MIRROR;
    sampler.addrW = rhi::TEXADDR_MIRROR;
    sampler.magFilter = rhi::TEXFILTER_LINEAR;
    sampler.minFilter = rhi::TEXFILTER_LINEAR;
    sampler.mipFilter = rhi::TEXMIPFILTER_NONE;
    return sampler;
}

}