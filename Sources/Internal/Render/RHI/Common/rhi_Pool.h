/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __RHI_POOL_H__
#define __RHI_POOL_H__

#include "../rhi_Type.h"
#include "Concurrency/Spinlock.h"
#include "MemoryManager/MemoryProfiler.h"

namespace rhi
{
enum
{
    HANDLE_INDEX_MASK = 0x0000FFFFU,
    HANDLE_INDEX_SHIFT = 0,

    HANDLE_GENERATION_MASK = 0x00FF0000U,
    HANDLE_GENERATION_SHIFT = 16,

    HANDLE_TYPE_MASK = 0xFF000000U,
    HANDLE_TYPE_SHIFT = 24,

    HANDLE_FORCEUINT32 = 0xFFFFFFFFU
};

#define RHI_HANDLE_INDEX(h) ((h & HANDLE_INDEX_MASK) >> HANDLE_INDEX_SHIFT)

template <class T, ResourceType RT, typename DT, bool need_restore = false>
class
ResourcePool
{
    struct Entry;

public:
    static Handle Alloc();
    static void Free(Handle h);

    static T* Get(Handle h);

    static void Reserve(unsigned maxCount);
    static unsigned ReCreateAll();

    class
    Iterator
    {
    public:
        void operator++()
        {
            do
            {
                ++entry;
            } while (entry != end && !entry->allocated);
        }
        T* operator->()
        {
            return &(entry->object);
        }
        T& operator*()
        {
            return entry->object;
        }

        bool operator!=(const Iterator& i)
        {
            return i.entry != this->entry;
        }

    private:
        friend class ResourcePool<T, RT, DT, need_restore>;

        Iterator(Entry* e, Entry* e_end)
            : entry(e)
            , end(e_end)
        {
        }

        Entry* entry;
        Entry* end;
    };

    static Iterator Begin();
    static Iterator End();

private:
    struct Entry
    {
        T object;

        uint32 allocated : 1;
        uint32 generation : 8;
        uint32 nextObjectIndex : 16;
    };

    static Entry* Object;
    static uint32 ObjectCount;
    static uint32 HeadIndex;
    static DAVA::Spinlock ObjectSync;
};

#define RHI_IMPL_POOL(T, RT, DT, nr) \
template<> rhi::ResourcePool<T, RT, DT, nr>::Entry* rhi::ResourcePool<T, RT, DT, nr>::Object = 0;    \
template<> uint32 rhi::ResourcePool<T, RT, DT, nr>::ObjectCount = 2048; \
template<> uint32 rhi::ResourcePool<T, RT, DT, nr>::HeadIndex = 0;    \
template<> DAVA::Spinlock rhi::ResourcePool<T, RT, DT, nr>::ObjectSync = {};   \

#define RHI_IMPL_POOL_SIZE(T, RT, DT, nr, sz) \
template<> rhi::ResourcePool<T, RT, DT, nr>::Entry* rhi::ResourcePool<T, RT, DT, nr>::Object = 0;    \
template<> uint32 rhi::ResourcePool<T, RT, DT, nr>::ObjectCount = sz;   \
template<> uint32 rhi::ResourcePool<T, RT, DT, nr>::HeadIndex = 0;    \
template<> DAVA::Spinlock rhi::ResourcePool<T, RT, DT, nr>::ObjectSync = {};

//------------------------------------------------------------------------------

template <class T, ResourceType RT, class DT, bool nr>
inline void
ResourcePool<T, RT, DT, nr>::Reserve(unsigned maxCount)
{
    ObjectSync.Lock();

    DVASSERT(Object == nullptr);
    DVASSERT(maxCount < HANDLE_INDEX_MASK);
    ObjectCount = maxCount;

    ObjectSync.Unlock();
}

//------------------------------------------------------------------------------

template <class T, ResourceType RT, class DT, bool nr>
inline Handle
ResourcePool<T, RT, DT, nr>::Alloc()
{
    uint32 handle = InvalidHandle;

    ObjectSync.Lock();

    if (!Object)
    {
        DAVA_MEMORY_PROFILER_ALLOC_SCOPE(DAVA::ALLOC_POOL_RHI_RESOURCE_POOL);
        Object = new Entry[ObjectCount];

        uint32 nextObjectIndex = 0;
        for (Entry *e = Object, *e_end = Object + ObjectCount; e != e_end; ++e)
        {
            ++nextObjectIndex;

            e->allocated = false;
            e->generation = 0;
            e->nextObjectIndex = nextObjectIndex;
        }

        (Object + ObjectCount - 1)->nextObjectIndex = 0;
    }

    Entry* e = Object + HeadIndex;
    DVASSERT(!e->allocated);
    HeadIndex = e->nextObjectIndex;

    e->allocated = true;
    ++e->generation;

    handle = 0;
    handle = ((uint32(e - Object) << HANDLE_INDEX_SHIFT) & HANDLE_INDEX_MASK) |
    (((e->generation) << HANDLE_GENERATION_SHIFT) & HANDLE_GENERATION_MASK) |
    ((RT << HANDLE_TYPE_SHIFT) & HANDLE_TYPE_MASK);

    ObjectSync.Unlock();

    DVASSERT(handle != InvalidHandle);

    return handle;
}

//------------------------------------------------------------------------------

template <class T, ResourceType RT, typename DT, bool nr>
inline void
ResourcePool<T, RT, DT, nr>::Free(Handle h)
{
    uint32 index = (h & HANDLE_INDEX_MASK) >> HANDLE_INDEX_SHIFT;
    uint32 type = (h & HANDLE_TYPE_MASK) >> HANDLE_TYPE_SHIFT;
    DVASSERT(type == RT);
    DVASSERT(index < ObjectCount);

    Entry* e = Object + index;
    DVASSERT(e->allocated);

    ObjectSync.Lock();

    e->nextObjectIndex = HeadIndex;
    HeadIndex = index;
    e->allocated = false;

    ObjectSync.Unlock();
}

//------------------------------------------------------------------------------

template <class T, ResourceType RT, typename DT, bool nr>
inline T* ResourcePool<T, RT, DT, nr>::Get(Handle h)
{
    DVASSERT(h != InvalidHandle);
    DVASSERT(((h & HANDLE_TYPE_MASK) >> HANDLE_TYPE_SHIFT) == RT);
    uint32 index = (h & HANDLE_INDEX_MASK) >> HANDLE_INDEX_SHIFT;
    DVASSERT(index < ObjectCount);
    Entry* e = Object + index;
    DVASSERT(e->allocated);
    DVASSERT(e->generation == ((h & HANDLE_GENERATION_MASK) >> HANDLE_GENERATION_SHIFT));

    return &(e->object);
}

//------------------------------------------------------------------------------

template <class T, ResourceType RT, typename DT, bool nr>
inline typename ResourcePool<T, RT, DT, nr>::Iterator
ResourcePool<T, RT, DT, nr>::Begin()
{
    return (Object) ? Iterator(Object, Object + ObjectCount) : Iterator(nullptr, nullptr);
}

//------------------------------------------------------------------------------

template <class T, ResourceType RT, typename DT, bool nr>
inline typename ResourcePool<T, RT, DT, nr>::Iterator
ResourcePool<T, RT, DT, nr>::End()
{
    return (Object) ? Iterator(Object + ObjectCount, Object + ObjectCount) : Iterator(nullptr, nullptr);
}

//------------------------------------------------------------------------------

template <class T, ResourceType RT, typename DT, bool nr>
inline unsigned
ResourcePool<T, RT, DT, nr>::ReCreateAll()
{
    unsigned count = 0;

    for (Iterator i = Begin(), i_end = End(); i != i_end; ++i)
    {
        DT desc = i->CreationDesc();

        i->Destroy(true);
        i->Create(desc, true);
        i->MarkNeedRestore();
        ++count;
    }

    return count;
}

//------------------------------------------------------------------------------

template <class T, class DT>
class
ResourceImpl
{
public:
    ResourceImpl()
        : needRestore(false)
    {
    }

    bool NeedRestore() const
    {
        return needRestore;
    }
    const DT& CreationDesc() const
    {
        return creationDesc;
    }

    void UpdateCreationDesc(const DT& desc)
    {
        creationDesc = desc;
    }
    void MarkNeedRestore()
    {
        if (!needRestore && creationDesc.needRestore)
        {
            needRestore = true;
            ++needRestoreCount;
        }
    }
    void MarkRestored()
    {
        if (needRestore)
        {
            needRestore = false;
            DVASSERT(needRestoreCount);
            --needRestoreCount;
        }
    }

    static unsigned NeedRestoreCount()
    {
        return needRestoreCount;
    }

private:
    DT creationDesc;
    bool needRestore;
    static unsigned needRestoreCount;
};

#define RHI_IMPL_RESOURCE(T, DT) \
template<> unsigned rhi::ResourceImpl<T, DT>::needRestoreCount = 0;

} // namespace rhi
#endif // __RHI_POOL_H__
