#if !defined __MEM_BUFFERALLOCATOR_HPP__
#define __MEM_BUFFERALLOCATOR_HPP__

#if !(TARGET_IPHONE_SIMULATOR == 1)

#include "../Common/rhi_RemoteHeap.h"
#include "_metal.h"
#include <string>

#define RHI_METAL__USE_BUF_PURGABLE_STATE 0

/*
struct
BufferTraits
{
    BufUID; // expected to be typedef'ed to proper render-API type (id<MTLBuffer>, IDirect3DVertexBuffer*, GLuint  etc.)

    static BufUID   Create( unsigned size );
    static void     Delete( BufUID uid );
    static void     Update( BufUID uid, unsigned offset, const void* data, unsigned data_size );
};
*/

namespace rhi
{
struct
MetalBufferTraits
{
    typedef id<MTLBuffer> BufUID;

    static BufUID Create(unsigned size)
    {
        id<MTLBuffer> buf = [_Metal_Device newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];

        [buf retain];
        return buf;
    }
    static void Delete(BufUID uid)
    {
        [uid release];
        [uid release];
        uid = nil;
    }
    static void Update(BufUID uid, unsigned offset, const void* data, unsigned data_size)
    {
        uint8* buf = (uint8*)([uid contents]);

        memcpy(buf + offset, data, data_size);
    }
};

template <class T>
class
BufferAllocator
{
public:
    struct
    Block
    {
        typename T::BufUID uid;
        unsigned base; // offset from start in buffer
        unsigned size;

        void Update(const void* data)
        {
            T::Update(uid, base, data, size);
        }
    };

    BufferAllocator(const char* name, unsigned page_size = 2 * 1024 * 1024, unsigned granularity = 1024);
    ~BufferAllocator();

    bool alloc(unsigned size, Block* block);
    bool free(const Block& block);

    void dump_stats() const;

    // disable copying
    BufferAllocator(const BufferAllocator&) = delete;
    BufferAllocator& operator=(const BufferAllocator&) = delete;

private:
    struct
    page_t
    {
        unsigned size;
        typename T::BufUID bufferUid;
        SimpleRemoteHeap<8 * 1024>* heap;
    };

    std::vector<page_t> _page;
    std::string _name;
    const unsigned _page_sz;
    const unsigned _granularity;
};

static void* MemBase = (void*)0xF0;

typedef BufferAllocator<MetalBufferTraits> MetalBufferAllocator;

//------------------------------------------------------------------------------

template <class T>
inline
BufferAllocator<T>::BufferAllocator(const char* name, unsigned page_size, unsigned granularity)
    : _name(name)
    ,
    _page_sz(page_size)
    ,
    _granularity(granularity)
{
    _page.reserve(8);
}

//------------------------------------------------------------------------------

template <class T>
inline
BufferAllocator<T>::~BufferAllocator()
{
}

//------------------------------------------------------------------------------

template <class T>
inline bool
BufferAllocator<T>::alloc(unsigned size, BufferAllocator<T>::Block* block)
{
    bool success = false;
    void* mem = nullptr;

    for (unsigned p = 0; p != _page.size(); ++p)
    {
        mem = _page[p].heap->alloc(L_ALIGNED_SIZE(size, _granularity));

        if (mem)
        {
            block->uid = _page[p].bufferUid;
            block->base = (uint8*)mem - (uint8*)MemBase;
            block->size = size;

            success = true;
            break;
        }
    }

    if (!mem)
    {
        page_t page;

        page.size = _page_sz;
        while (page.size < size)
            page.size *= 2;
        page.bufferUid = T::Create(page.size);

        page.heap = new SimpleRemoteHeap<8 * 1024>();
        page.heap->initialize(MemBase, page.size);

        _page.push_back(page);
        DAVA::Logger::Debug("\"%s\" allocated page (%u in total)", _name.c_str(), _page.size());

        mem = page.heap->alloc(L_ALIGNED_SIZE(size, _granularity));

        if (mem)
        {
            block->uid = page.bufferUid;
            block->base = (uint8*)mem - (uint8*)MemBase;
            block->size = size;

            success = true;
        }
    }

    return success;
}

//------------------------------------------------------------------------------

template <class T>
inline bool
BufferAllocator<T>::free(const BufferAllocator<T>::Block& block)
{
    bool success = false;

    for (typename std::vector<page_t>::iterator p = _page.begin(), p_end = _page.end(); p != p_end; ++p)
    {
        if (block.uid == p->bufferUid)
        {
            p->heap->free((uint8*)MemBase + block.base);
            success = true;

            // de-allocate entire page, if possible
            if (!p->heap->has_allocated_blocks())
            {
                p->heap->uninitialize();
                delete p->heap;

                T::Delete(p->bufferUid);

                _page.erase(p);
                DAVA::Logger::Debug("\"%s\" de-allocated page (%u in total)", _name.c_str(), _page.size());
            }

            break;
        }
    }

    return success;
}

//------------------------------------------------------------------------------

template <class T>
inline void
BufferAllocator<T>::dump_stats() const
{
    DAVA::Logger::Info("\"%s\" pages (%u)", _name.c_str(), _page.size());
    for (unsigned p = 0; p != _page.size(); ++p)
    {
        RemoteHeapBase::Stat s;

        _page[p].heap->get_stats(&s);
        DAVA::Logger::Info("  [%u] heap usage : %.1f%% (%u blocks)  comitted %.1f%%", p, 100.0f * (float(s.alloced_size) / float(s.reserved)), s.alloced_block_count, 100.0f * (float(s.comitted_size) / float(s.reserved)));
    }
}
}

#endif // !(TARGET_IPHONE_SIMULATOR == 1)
#endif // __MEM_BUFFERALLOCATOR_HPP__
