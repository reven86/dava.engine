//
//  STLBasePoolAllocator.h
//  Framework
//
//  Created by Sergey Bogdanov on 3/7/14.
//
//

#ifndef __Framework__STLBasePoolAllocator__
#define __Framework__STLBasePoolAllocator__

#include <istream>
#include "Base/STLPoolAllocatorFactory.h"



template <class T> class STLBasePoolAllocator
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
        typedef STLBasePoolAllocator<U> other;
    };
    
    STLBasePoolAllocator()
    {
    }
    STLBasePoolAllocator(const STLBasePoolAllocator&)
    {
    }
    template <class U>
    STLBasePoolAllocator(const STLBasePoolAllocator<U>&)
    {
    }
    ~STLBasePoolAllocator()
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
        
        if (DAVA::STLPoolAllocatorFactory::Instance() )
        {
            
            return static_cast<pointer>(DAVA::STLPoolAllocatorFactory::Instance()->Allocate(sizeof(T),n,true));
        }
        
        void* p = std::malloc(n * sizeof(T));
        if (!p)
        {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(p);
    }
    
    void deallocate(pointer p, size_type size)
    {
        if (DAVA::STLPoolAllocatorFactory::Instance() )
        {
            return DAVA::STLPoolAllocatorFactory::Instance()->Deallocate(p,true);
        }
        std::free(p);
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
    void operator=(const STLBasePoolAllocator&);
};

template<> class STLBasePoolAllocator<void>
{
    typedef void        value_type;
    typedef void*       pointer;
    typedef const void* const_pointer;
    
    template <class U>
    struct rebind
    {
        typedef STLBasePoolAllocator<U> other;
    };
};


template <class T>
inline bool operator==(const STLBasePoolAllocator<T>&,
                       const STLBasePoolAllocator<T>&)
{
    return true;
}

template <class T>
inline bool operator!=(const STLBasePoolAllocator<T>&,
                       const STLBasePoolAllocator<T>&)
{
    return false;
}



#endif /* defined(__Framework__STLBasePoolAllocator__) */
