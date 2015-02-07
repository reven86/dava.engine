#pragma once

#include "memprof_config.h"

#if defined(MEMPROF_ENABLE)

#include <limits>

#include "mem_profiler.h"
#include "internal_alloc.h"

template<typename T>
class profiled_allocator
{
public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template <typename U>
    struct rebind
    {
        typedef profiled_allocator<U> other;
    };

    profiled_allocator() {}
    profiled_allocator(const profiled_allocator&) = default;
    profiled_allocator& operator = (const profiled_allocator&) = delete;
    template <typename U>
    profiled_allocator(const profiled_allocator<U>&) {}
    ~profiled_allocator() = default;

    T* address(T& r) const { return &r; }
    const T* address(const T& s) const { return &s; }
    size_t max_size() const { return std::numeric_limits<size_t>::max() / sizeof(T); }

    bool operator == (const profiled_allocator& other) const { return true; }
    bool operator != (const profiled_allocator& other) const { return !(*this == other); }

    void construct(T* const p, const T& t) const
    {
        void* const buf = static_cast<void *>(p);
        new (buf) T(t);
    }

    void destroy(T* const p) const
    {
        p->~T();
    }

    T* allocate(const size_t n) const
    {
        // TODO: check for n == 0, n > max_size(), out of memory
        void* ptr = internal_alloc(n * sizeof(T), mem_type_e::MEM_TYPE_STL);
        return static_cast<T*>(ptr);
    }

    void deallocate(T* const p, const size_t n) const
    {
        internal_free(p);
    }

    template <typename U>
    T* allocate(const size_t n, const U* /* const hint */) const { return allocate(n); }

private:
};

#endif
