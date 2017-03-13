#pragma once
//==============================================================================
//
//  Heap that keeps book-keeping info in main-memory (NOT in the memory managed by heap)
//  Good for managing GPU local memory, audio-memory etc.
//
//==============================================================================
//
//  externals:

#include <vector>
#include "../Common/rhi_Utils.h"
#include "Concurrency/LockGuard.h"

#define RHI_THREAD_SAFE_REMOTE_HEAP 1

class
RemoteHeapBase
{
public:
    struct
    Stat
    {
        unsigned reserved;
        unsigned comitted_size;
        unsigned alloced_size;
        unsigned alloced_block_count;
        unsigned total_block_count;
    };
};

template <unsigned MaxBlockCount>
class
SimpleRemoteHeap
: public RemoteHeapBase
{
public:
    SimpleRemoteHeap(const char* name = "<remote heap>");
    ~SimpleRemoteHeap();

    void initialize(void* base, unsigned size, unsigned align = 8);
    void uninitialize();

    void* alloc(unsigned sz);
    void* alloc_aligned(unsigned sz, unsigned align);
    void free(void* mem);

    bool has_allocated_blocks() const;
    void dump_stats() const;
    void get_stats(Stat* stat) const;

private:
    struct Block;
    typedef std::vector<Block> BlockList;
    typedef typename std::vector<Block>::iterator BlockList_Iter;
    typedef typename std::vector<Block>::const_iterator BlockList_ConstIter;

    Block* _commit_block(unsigned sz, unsigned align);
    Block* _find_free_block(unsigned sz, unsigned align, unsigned* slack = nullptr);
    void _defragment();

    uint8_t* _base;
    uint8_t* _unused;
    unsigned _total_size;
    unsigned _block_align;
    const char* _name; // ref-only, purely for debug purposes

    struct
    Block
    {
        enum
        {
            Allocated = 0x00000001,

            Force32 = 0xFFFFFFFF
        };

        uint8_t* base; // 'real' address
        uint32_t size; // 'real', aligned size
        uint32_t align;
        uint8_t* usr_ptr; // ponter given outside of allocator
        uint32_t usr_sz;
        uint32_t slack;
        uint32_t flags;

        bool is_alloced() const
        {
            return flags & Allocated;
        }

        void mark_alloced()
        {
            flags |= Allocated;
        }
        void mark_not_alloced()
        {
            flags &= ~Allocated;
        }
    };

    BlockList _block;

#if (RHI_THREAD_SAFE_REMOTE_HEAP)
    /* _lock is declared mutable in order to have an ability to lock mutex inside const method */
    mutable DAVA::Mutex _lock; 
#endif
};

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline
SimpleRemoteHeap<MaxBlockCount>::SimpleRemoteHeap(const char* name)
    : _base(nullptr)
    ,
    _unused(nullptr)
    ,
    _total_size(0)
    ,
    _block_align(0)
    ,
    _name(name)
{
    _block.reserve(MaxBlockCount);
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline
SimpleRemoteHeap<MaxBlockCount>::~SimpleRemoteHeap()
{
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void
SimpleRemoteHeap<MaxBlockCount>::initialize(void* base, unsigned size, unsigned align)
{
    _base = reinterpret_cast<uint8_t*>(base);
    _unused = reinterpret_cast<uint8_t*>(base);
    _total_size = size;
    _block_align = align;
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void
SimpleRemoteHeap<MaxBlockCount>::uninitialize()
{
    _base = nullptr;
    _unused = nullptr;
    _total_size = 0;
    _block_align = 0;
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void*
SimpleRemoteHeap<MaxBlockCount>::alloc(unsigned size)
{
    DVASSERT(size > 0);

    if (size > _total_size)
        return nullptr;

#if (RHI_THREAD_SAFE_REMOTE_HEAP)
    DAVA::LockGuard<DAVA::Mutex> lock(_lock);
#endif

    _defragment();

    uint8_t* mem = nullptr;
    unsigned slack = 0;
    Block* block = _find_free_block(size, _block_align, &slack);

    if (block)
        block->slack = slack;
    else
        block = _commit_block(size, _block_align);

    if (block)
    {
        mem = block->usr_ptr;
        block->mark_alloced();
    }

    return mem;
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void*
SimpleRemoteHeap<MaxBlockCount>::alloc_aligned(unsigned size, unsigned align)
{
#if (RHI_THREAD_SAFE_REMOTE_HEAP)
    DAVA::LockGuard<DAVA::Mutex> lock(_lock);
#endif

    _defragment();

    uint8_t* mem = nullptr;
    unsigned slack = 0;
    Block* block = _find_free_block(size, align, &slack);

    if (block)
        block->slack = slack;
    else
        block = _commit_block(size, align);

    if (block)
    {
        mem = block->usr_ptr;
        block->mark_alloced();
    }

    return mem;
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void
SimpleRemoteHeap<MaxBlockCount>::free(void* mem)
{
    uint8_t* usr_ptr = reinterpret_cast<uint8_t*>(mem);
    DVASSERT(usr_ptr >= _base);
    DVASSERT(usr_ptr < _base + _total_size);

#if (RHI_THREAD_SAFE_REMOTE_HEAP)
    DAVA::LockGuard<DAVA::Mutex> lock(_lock);
#endif

    for (BlockList_Iter b = _block.begin(), b_end = _block.end(); b != b_end; ++b)
    {
        if (b->usr_ptr == usr_ptr)
        {
            DVASSERT(b->is_alloced());
            b->mark_not_alloced();
            b->slack = 0;
            break;
        }
    }
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline typename SimpleRemoteHeap<MaxBlockCount>::Block*
SimpleRemoteHeap<MaxBlockCount>::_commit_block(unsigned size, unsigned align)
{
    // allocate new block from 'unused' region

    DVASSERT(_base);

    Block* block = nullptr;
    unsigned sz = L_ALIGNED_SIZE(size, align);

    if ((_unused + sz <= _base + _total_size) && (_block.size() < MaxBlockCount))
    {
        _block.emplace_back();

        Block& b = _block.back();
        b.base = _unused;
        b.size = sz;
        b.align = align;
        b.usr_ptr = reinterpret_cast<uint8_t*>((reinterpret_cast<uint64_t>(b.base) + (uint64_t(align) - 1)) & (~(uint64_t(align) - 1)));
        b.usr_sz = size;
        b.slack = 0;
        b.flags = 0;
        block = &b;

        _unused += sz;
    }

    return block;
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline typename SimpleRemoteHeap<MaxBlockCount>::Block*
SimpleRemoteHeap<MaxBlockCount>::_find_free_block(unsigned size, unsigned align, unsigned* wasted_slack)
{
    BlockList_Iter block = _block.end();
    unsigned min_slack = size;

    unsigned sz = L_ALIGNED_SIZE(size, align);

    for (BlockList_Iter b = _block.begin(), b_end = _block.end(); b != b_end; ++b)
    {
        if (!b->is_alloced() && b->usr_sz >= sz)
        {
            unsigned slack = b->usr_sz - size;

            if (slack < min_slack)
            {
                block = b;
                min_slack = slack;
            }
        }
    }

    if (block != _block.end())
    {
        if (wasted_slack)
            *wasted_slack = min_slack;
    }

    return (block != _block.end()) ? &(_block[block - _block.begin()]) : nullptr;
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void
SimpleRemoteHeap<MaxBlockCount>::_defragment()
{
    bool has_free_blk = false;
    bool do_wipe = true;

    for (BlockList_ConstIter b = _block.begin(), b_end = _block.end(); b != b_end; ++b)
    {
        if (b->is_alloced())
        {
            do_wipe = false;
            break;
        }
    }

    if (do_wipe)
    {
        _block.clear();
        _unused = _base;
        return;
    }

    do
    {
        uint8_t* max_ptr = _base;
        BlockList_Iter blk = _block.end();

        for (BlockList_Iter b = _block.begin(), b_end = _block.end(); b != b_end; ++b)
        {
            if (b->base > max_ptr)
            {
                blk = b;
                max_ptr = b->base;
            }
        }

        if ((blk != _block.end()) && !blk->is_alloced())
        {
            // de-commit block's memory
            _unused -= blk->size;
            _block.erase(blk);

            has_free_blk = true;
        }
        else
        {
            has_free_blk = false;
        }
    }
    while (has_free_blk);
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline bool
SimpleRemoteHeap<MaxBlockCount>::has_allocated_blocks() const
{
    bool has = false;

#if (RHI_THREAD_SAFE_REMOTE_HEAP)
    DAVA::LockGuard<DAVA::Mutex> lock(_lock);
#endif

    for (BlockList_ConstIter b = _block.begin(), b_end = _block.end(); b != b_end; ++b)
    {
        if (b->is_alloced())
        {
            has = true;
            break;
        }
    }

    return has;
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void
SimpleRemoteHeap<MaxBlockCount>::get_stats(RemoteHeapBase::Stat* stat) const
{
    unsigned wasted_align = 0;
    unsigned wasted_slack = 0;
    unsigned alloced = 0;

    stat->alloced_size = 0;
    stat->alloced_block_count = 0;

#if (RHI_THREAD_SAFE_REMOTE_HEAP)
    DAVA::LockGuard<DAVA::Mutex> lock(_lock);
#endif

    for (BlockList_ConstIter b = _block.begin(), b_end = _block.end(); b != b_end; ++b)
    {
        wasted_align += b->size - b->usr_sz;
        wasted_slack += b->slack;

        if (b->is_alloced())
        {
            stat->alloced_size += b->usr_sz;
            ++stat->alloced_block_count;
        }
    }

    stat->reserved = _total_size;
    stat->comitted_size = _unused - _base;
    stat->total_block_count = _block.size();
}

//------------------------------------------------------------------------------

template <unsigned MaxBlockCount>
inline void
SimpleRemoteHeap<MaxBlockCount>::dump_stats() const
{
    unsigned wasted_align = 0;
    unsigned wasted_slack = 0;
    unsigned alloced = 0;
    unsigned free_block_cnt = 0;
    unsigned free_sz = 0;
    unsigned comitted = _unused - _base;

#if (RHI_THREAD_SAFE_REMOTE_HEAP)
    DAVA::LockGuard<DAVA::Mutex> lock(_lock);
#endif

    for (BlockList_ConstIter b = _block.begin(), b_end = _block.end(); b != b_end; ++b)
    {
        wasted_align += b->size - b->usr_sz;
        wasted_slack += b->slack;

        if (b->is_alloced())
        {
            alloced += b->usr_sz;
        }
        else
        {
            ++free_block_cnt;
            free_sz += b->usr_sz;
        }
    }
    /*
    Trace( "\"%s\" stats\n", _name );
    Trace( "reserved : %s\n", FormattedInt(_total_size).text() );
    Trace( "comitted : %s (%i%%)\n", FormattedInt(comitted).text(), int(100.0f*float(comitted)/float(_total_size)) );
    Trace( "alloced  : %s\n", FormattedInt(alloced).text() );
    Trace( "free     : %s (%u blocks)\n", FormattedInt(free_sz).text(), free_block_cnt );
    Trace( "wasted   : %s (%s + %s)\n", FormattedInt(wasted_align+wasted_slack).text(), FormattedInt(wasted_align).text(), FormattedInt(wasted_slack).text() );
    Trace( "overhead : %s\n", FormattedInt(sizeof(SimpleRemoteHeap<MaxBlockCount>)).text() );
    Trace( "blocks   : %u\n", _block.count() );
*/
}
