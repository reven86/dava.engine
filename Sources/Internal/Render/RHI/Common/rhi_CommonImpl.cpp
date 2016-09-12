#include "../rhi_Public.h"
#include "rhi_Private.h"
#include "rhi_Pool.h"

#include "rhi_CommonImpl.h"
namespace rhi
{
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

typedef ResourcePool<TextureSet_t, RESOURCE_TEXTURE_SET, TextureSet_t::Desc, false> TextureSetPool;
RHI_IMPL_POOL(TextureSet_t, RESOURCE_TEXTURE_SET, TextureSet_t::Desc, false);

struct TextureSetInfo
{
    TextureSetDescriptor desc;
    Handle handle;
};

struct DepthStencilState_t
{
    DepthStencilState::Descriptor desc;
    Handle state;
    int refCount;
};

struct SamplerState_t
{
    SamplerState::Descriptor desc;
    Handle state;
    int refCount;
};

static DAVA::Mutex _TextureSetInfoMutex;
static std::vector<TextureSetInfo> _TextureSetInfo;

static DAVA::Mutex _DepthStencilStateInfoMutex;
static std::vector<DepthStencilState_t> _DepthStencilStateInfo;

static DAVA::Mutex _SamplerStateInfoMutex;
static std::vector<SamplerState_t> _SamplerStateInfo;

void InitTextreSetPool(uint32 maxCount)
{
    TextureSetPool::Reserve(maxCount);
}

HTextureSet AcquireTextureSet(const TextureSetDescriptor& desc)
{
    HTextureSet handle;

    DAVA::LockGuard<DAVA::Mutex> lock(_TextureSetInfoMutex);
    for (std::vector<TextureSetInfo>::const_iterator i = _TextureSetInfo.begin(), i_end = _TextureSetInfo.end(); i != i_end; ++i)
    {
        if (i->desc.fragmentTextureCount == desc.fragmentTextureCount && i->desc.vertexTextureCount == desc.vertexTextureCount && memcmp(i->desc.fragmentTexture, desc.fragmentTexture, desc.fragmentTextureCount * sizeof(Handle)) == 0 && memcmp(i->desc.vertexTexture, desc.vertexTexture, desc.vertexTextureCount * sizeof(Handle)) == 0)
        {
            TextureSet_t* ts = TextureSetPool::Get(i->handle);

            ++ts->refCount;

            handle = HTextureSet(i->handle);
            break;
        }
    }

    if (!handle.IsValid())
    {
        handle = HTextureSet(TextureSetPool::Alloc());

        TextureSet_t* ts = TextureSetPool::Get(handle);
        TextureSetInfo info;

        ts->refCount = 1;
        ts->fragmentTextureCount = desc.fragmentTextureCount;
        ts->vertexTextureCount = desc.vertexTextureCount;
        memcpy(ts->fragmentTexture, desc.fragmentTexture, desc.fragmentTextureCount * sizeof(Handle));
        memcpy(ts->vertexTexture, desc.vertexTexture, desc.vertexTextureCount * sizeof(Handle));

        info.desc = desc;
        info.handle = handle;
        _TextureSetInfo.push_back(info);
    }

    return handle;
}

//------------------------------------------------------------------------------

HTextureSet CopyTextureSet(HTextureSet tsh)
{
    HTextureSet handle;
    TextureSet_t* ts = TextureSetPool::Get(tsh);

    if (ts)
    {
        ++ts->refCount;
        handle = tsh;
    }

    return handle;
}

//------------------------------------------------------------------------------

void ReleaseTextureSet(HTextureSet tsh, bool forceImmediate)
{
    if (tsh != InvalidHandle)
    {
        TextureSet_t* ts = TextureSetPool::Get(tsh);
        if (--ts->refCount == 0)
        {
            if (forceImmediate)
                TextureSetPool::Free(tsh);
            else
                ScheduleResourceDeletion(tsh, RESOURCE_TEXTURE_SET);

            DAVA::LockGuard<DAVA::Mutex> lock(_TextureSetInfoMutex);
            for (std::vector<TextureSetInfo>::iterator i = _TextureSetInfo.begin(), i_end = _TextureSetInfo.end(); i != i_end; ++i)
            {
                if (i->handle == tsh)
                {
                    _TextureSetInfo.erase(i);
                    break;
                }
            }
        }
    }
}

void ReplaceTextureInAllTextureSets(HTexture oldHandle, HTexture newHandle)
{
    DAVA::LockGuard<DAVA::Mutex> lock(_TextureSetInfoMutex);
    for (std::vector<TextureSetInfo>::iterator s = _TextureSetInfo.begin(), s_end = _TextureSetInfo.end(); s != s_end; ++s)
    {
        // update texture-set itself

        TextureSet_t* ts = TextureSetPool::Get(s->handle);

        if (ts)
        {
            for (unsigned i = 0; i != ts->fragmentTextureCount; ++i)
            {
                if (ts->fragmentTexture[i] == oldHandle)
                    ts->fragmentTexture[i] = newHandle;
            }
            for (unsigned i = 0; i != ts->vertexTextureCount; ++i)
            {
                if (ts->vertexTexture[i] == oldHandle)
                    ts->vertexTexture[i] = newHandle;
            }
        }

        // update desc as well

        for (uint32 t = 0; t != s->desc.fragmentTextureCount; ++t)
        {
            if (s->desc.fragmentTexture[t] == oldHandle)
                s->desc.fragmentTexture[t] = newHandle;
        }
        for (uint32 t = 0; t != s->desc.vertexTextureCount; ++t)
        {
            if (s->desc.vertexTexture[t] == oldHandle)
                s->desc.vertexTexture[t] = newHandle;
        }
    }
}

//------------------------------------------------------------------------------

HDepthStencilState AcquireDepthStencilState(const DepthStencilState::Descriptor& desc)
{
    Handle ds = InvalidHandle;
    DAVA::LockGuard<DAVA::Mutex> lock(_DepthStencilStateInfoMutex);
    for (std::vector<DepthStencilState_t>::iterator i = _DepthStencilStateInfo.begin(), i_end = _DepthStencilStateInfo.end(); i != i_end; ++i)
    {
        if (memcmp(&(i->desc), &desc, sizeof(DepthStencilState::Descriptor)) == 0)
        {
            ds = i->state;
            ++i->refCount;
            break;
        }
    }

    if (ds == InvalidHandle)
    {
        DepthStencilState_t info;

        info.desc = desc;
        info.state = DepthStencilState::Create(desc);
        info.refCount = 1;

        _DepthStencilStateInfo.push_back(info);
        ds = info.state;
    }

    return HDepthStencilState(ds);
}

//------------------------------------------------------------------------------

HDepthStencilState CopyDepthStencilState(HDepthStencilState ds)
{
    HDepthStencilState handle;

    DAVA::LockGuard<DAVA::Mutex> lock(_DepthStencilStateInfoMutex);
    for (std::vector<DepthStencilState_t>::iterator i = _DepthStencilStateInfo.begin(), i_end = _DepthStencilStateInfo.end(); i != i_end; ++i)
    {
        if (i->state == ds)
        {
            ++i->refCount;
            handle = ds;
            break;
        }
    }

    return handle;
}

//------------------------------------------------------------------------------

void ReleaseDepthStencilState(HDepthStencilState ds, bool forceImmediate)
{
    DAVA::LockGuard<DAVA::Mutex> lock(_DepthStencilStateInfoMutex);
    for (std::vector<DepthStencilState_t>::iterator i = _DepthStencilStateInfo.begin(), i_end = _DepthStencilStateInfo.end(); i != i_end; ++i)
    {
        if (i->state == ds)
        {
            if (--i->refCount == 0)
            {
                if (forceImmediate)
                    DepthStencilState::Delete(i->state);
                else
                    ScheduleResourceDeletion(i->state, RESOURCE_DEPTHSTENCIL_STATE);
                _DepthStencilStateInfo.erase(i);
            }

            break;
        }
    }
}

//------------------------------------------------------------------------------

HSamplerState AcquireSamplerState(const SamplerState::Descriptor& desc)
{
    Handle ss = InvalidHandle;

    DAVA::LockGuard<DAVA::Mutex> lock(_SamplerStateInfoMutex);
    for (std::vector<SamplerState_t>::iterator i = _SamplerStateInfo.begin(), i_end = _SamplerStateInfo.end(); i != i_end; ++i)
    {
        if (memcmp(&(i->desc), &desc, sizeof(SamplerState::Descriptor)) == 0)
        {
            ss = i->state;
            ++i->refCount;
            break;
        }
    }

    if (ss == InvalidHandle)
    {
        SamplerState_t info;

        info.desc = desc;
        info.state = SamplerState::Create(desc);
        info.refCount = 1;

        _SamplerStateInfo.push_back(info);
        ss = info.state;
    }

    return HSamplerState(ss);
}

//------------------------------------------------------------------------------

HSamplerState CopySamplerState(HSamplerState ss)
{
    Handle handle = InvalidHandle;

    DAVA::LockGuard<DAVA::Mutex> lock(_SamplerStateInfoMutex);
    for (std::vector<SamplerState_t>::iterator i = _SamplerStateInfo.begin(), i_end = _SamplerStateInfo.end(); i != i_end; ++i)
    {
        if (i->state == ss)
        {
            ++i->refCount;
            handle = i->state;
            break;
        }
    }

    return HSamplerState(handle);
}

//------------------------------------------------------------------------------

void ReleaseSamplerState(HSamplerState ss, bool forceImmediate)
{
    DAVA::LockGuard<DAVA::Mutex> lock(_SamplerStateInfoMutex);
    for (std::vector<SamplerState_t>::iterator i = _SamplerStateInfo.begin(), i_end = _SamplerStateInfo.end(); i != i_end; ++i)
    {
        if (i->state == ss)
        {
            if (--i->refCount == 0)
            {
                if (forceImmediate)
                    SamplerState::Delete(i->state);
                else
                    ScheduleResourceDeletion(i->state, RESOURCE_SAMPLER_STATE);
                _SamplerStateInfo.erase(i);
            }

            break;
        }
    }
}
}