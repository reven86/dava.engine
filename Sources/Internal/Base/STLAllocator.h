//
//  STLAllocator.h
//  Framework
//
//  Created by Sergey Bogdanov on 3/7/14.
//
//

#ifndef __Framework__STLAllocator__
#define __Framework__STLAllocator__

#include <istream>
#include "Base/STLAllocatorFactory.h"



template <class T> class STLAllocator
{
public:
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;
    
    template <class U>
    struct rebind
    {
        typedef STLAllocator<U> other;
    };
    
    STLAllocator()
    {
    }
    STLAllocator(const STLAllocator&)
    {
    }
    template <class U>
    STLAllocator(const STLAllocator<U>&)
    {
    }
    ~STLAllocator()
    {
    }
    
    pointer address(reference x) const
    {
        return &x;
    }
    const_pointer address(const_reference x) const
    {
        return x;
    }
    
    pointer allocate(size_type n, const_pointer = 0)
    {
        static DAVA::FixedSizePoolAllocator * alloc = DAVA::STLAllocatorFactory::Instance()->GetAllocator(sizeof(T));
        /*void* p = std::malloc(n * sizeof(T));
        if (!p)
            throw std::bad_alloc();
        return static_cast<pointer>(p);*/
        return (pointer)alloc->New();
    }
    
    void deallocate(pointer p, size_type size)
    {
        static DAVA::FixedSizePoolAllocator * alloc = DAVA::STLAllocatorFactory::Instance()->GetAllocator(size);
        alloc->Delete(p);
        //std::free(p);
    }
    
    size_type max_size() const
    {
        return static_cast<size_type>(-1) / sizeof(T);
    }
    
    void construct(pointer p, const value_type& x)
    {
        new(p) value_type(x);
    }
    void destroy(pointer p)
    {
        p->~value_type();
    }
    
private:
    void operator=(const STLAllocator&);
};

template<> class STLAllocator<void>
{
    typedef void        value_type;
    typedef void*       pointer;
    typedef const void* const_pointer;
    
    template <class U>
    struct rebind
    {
        typedef STLAllocator<U> other;
    };
};


template <class T>
inline bool operator==(const STLAllocator<T>&,
                       const STLAllocator<T>&)
{
    return true;
}

template <class T>
inline bool operator!=(const STLAllocator<T>&,
                       const STLAllocator<T>&)
{
    return false;
}



#endif /* defined(__Framework__STLAllocator__) */
