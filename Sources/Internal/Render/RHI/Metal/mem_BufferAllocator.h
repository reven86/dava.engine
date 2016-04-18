#if !defined __MEM_BUFFERALLOCATOR_HPP__
#define __MEM_BUFFERALLOCATOR_HPP__

#include "mem_RemoteHeap.h"
#include "_metal.h"
#include <string>

namespace rhi
{
class
BufferAllocator
{
public:
    struct
    Block
    {
        id<MTLBuffer> buffer;
        void* ptr;
        unsigned base; // offset from start in buffer
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
        id<MTLBuffer> buffer;
        SimpleRemoteHeap<8 * 1024>* heap;
    };

    std::vector<page_t> _page;
    std::string _name;
    const unsigned _page_sz;
    const unsigned _granularity;
};

//------------------------------------------------------------------------------

inline
BufferAllocator::BufferAllocator(const char* name, unsigned page_size, unsigned granularity)
    : _name(name)
    ,
    _page_sz(page_size)
    ,
    _granularity(granularity)
{
    _page.reserve(8);
}

//------------------------------------------------------------------------------

inline
BufferAllocator::~BufferAllocator()
{
}

//------------------------------------------------------------------------------

inline bool
BufferAllocator::alloc(unsigned size, BufferAllocator::Block* block)
{
    bool success = false;
    void* mem = nullptr;

    for (unsigned p = 0; p != _page.size(); ++p)
    {
        mem = _page[p].heap->alloc(L_ALIGNED_SIZE(size, _granularity));

        if (mem)
        {
            block->buffer = _page[p].buffer;
            block->ptr = mem;
            block->base = (uint8*)(mem) - (uint8*)([_page[p].buffer contents]);

            success = true;
            break;
        }
    }

    if (!mem)
    {
        page_t page;

        page.size = _page_sz;
        page.buffer = [_Metal_Device newBufferWithLength:page.size options:MTLResourceOptionCPUCacheModeDefault];
        DVASSERT(page.buffer);
        [page.buffer retain];
        [page.buffer setPurgeableState:MTLPurgeableStateNonVolatile];

        page.heap = new SimpleRemoteHeap<8 * 1024>();
        page.heap->initialize([page.buffer contents], page.size);

        _page.push_back(page);
        DAVA::Logger::Info("\"%s\" allocated page (%u in total)", _name.c_str(), _page.size());

        mem = page.heap->alloc(L_ALIGNED_SIZE(size, _granularity));

        if (mem)
        {
            block->buffer = page.buffer;
            block->ptr = mem;
            block->base = (uint8*)(mem) - (uint8*)([page.buffer contents]);

            success = true;
        }
    }

    return success;
}

//------------------------------------------------------------------------------

inline bool
BufferAllocator::free(const BufferAllocator::Block& block)
{
    bool success = false;

    for (std::vector<page_t>::iterator p = _page.begin(), p_end = _page.end(); p != p_end; ++p)
    {
        if (block.buffer == p->buffer)
        {
            p->heap->free(block.ptr);
            success = true;

            // de-allocate entire page, if possible
            if (!p->heap->has_allocated_blocks())
            {
                p->heap->uninitialize();
                delete p->heap;

                [p->buffer setPurgeableState:MTLPurgeableStateEmpty];
                [p->buffer release];
                p->buffer = nil;

                _page.erase(p);
                DAVA::Logger::Info("\"%s\" de-allocated page (%u in total)", _name.c_str(), _page.size());
            }

            break;
        }
    }

    return success;
}

//------------------------------------------------------------------------------

inline void
BufferAllocator::dump_stats() const
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


#endif // __MEM_BUFFERALLOCATOR_HPP__
