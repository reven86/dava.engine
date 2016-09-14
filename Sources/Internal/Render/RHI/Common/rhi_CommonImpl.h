#pragma once
#include "../rhi_Type.h"

namespace rhi
{
namespace CommonImpl
{
struct Frame
{
    Handle sync = InvalidHandle;
    Handle perfQuerySet = InvalidHandle;
    std::vector<Handle> pass;
    bool readyToExecute = false;
    uint32 frameNumber;
};

struct ImmediateCommand
{
    void* cmdData = nullptr; //TODO - should be common immediate command interface like software command ?
    uint32 cmdCount;
    bool forceImmediate = false;
};

struct TextureSet_t
{
    struct Desc
    {
    };

    uint32 fragmentTextureCount;
    Handle fragmentTexture[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32 vertexTextureCount;
    Handle vertexTexture[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    int refCount;
};
}

namespace TextureSet
{
Handle Create();
CommonImpl::TextureSet_t* Get(Handle);
void Delete(Handle);
void InitTextreSetPool(uint32 maxCount);
}
}