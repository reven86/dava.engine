//
//  FixedStringAllocator.h
//  Framework
//
//  Created by Vladimir Bondarenko on 3/7/14.
//
//

#ifndef Framework_FixedStringAllocator_h
#define Framework_FixedStringAllocator_h

#if defined(__USE_FIXED_STRING_ALLOCATOR__)
#include <pthread.h>
#include <map>
#include "Debug/DVAssert.h"

namespace DAVA {
    
const int ALLOCATOR_BUFFER_SIZE = 32768;

struct AllocatorBuffer
{
    AllocatorBuffer() : currentPosition(0), pBuffer(0){}
    inline void* GetPointer(int size)
    {
        if ((size & 15) != 0)
        {
            size += 16 - (size & 15);
        }
        
        if(currentPosition + size >= ALLOCATOR_BUFFER_SIZE)
        {
            currentPosition = 0;
        }
        void* p = pBuffer + currentPosition;
        currentPosition+= size;
        return p;
    }
    
    char *pBuffer;
    int currentPosition;
};
    
class Pointer : public BaseObject
{
protected:
    ~Pointer() {};
public:
    Pointer() : pointer(NULL){}
    int32 Release()
    {
        int32 refCounter = AtomicDecrement(referenceCount);
		return refCounter;
    }
    
    inline void SetPointer(uint8 *p) { pointer = p; }
    inline uint8* GetPointer() { return pointer; }
    
private:
    uint8 * pointer;
};
    
Map<pthread_t, AllocatorBuffer*> fixedAllocatorBuffers;
    
AllocatorBuffer *GetAllocatorBuffer()
{
    pthread_t threadID = pthread_self();
    AllocatorBuffer *bufferAlloc = fixedAllocatorBuffers[threadID];
    if(!bufferAlloc)
    {
        bufferAlloc = new AllocatorBuffer();
        bufferAlloc->pBuffer = new char[ALLOCATOR_BUFFER_SIZE];
        fixedAllocatorBuffers[threadID] = bufferAlloc;
    }
    return bufferAlloc;
}
    
template <typename T>
class FixedAllocator
{
public:
    
    typedef T value_type;
    typedef value_type * pointer;
    typedef const value_type * const_pointer;
    typedef value_type & reference;
    typedef const value_type & const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    
    template<typename U>
    struct rebind
    {
        typedef FixedAllocator<U> other;
    };
    
    FixedAllocator()
    {
        AllocatorBuffer *bufferAlloc = GetAllocatorBuffer();
        void *p = bufferAlloc->GetPointer(sizeof(Pointer));
        buffer = RefPtr<Pointer>(new (p)Pointer());
    }
    
    FixedAllocator(const FixedAllocator &rhs)
    {
        buffer = rhs.buffer;
    }
    
    ~FixedAllocator()
    {
        buffer.Set(NULL);
    }
    
    pointer address(reference x) const
    {
        return (&x);
    }
    
    const_pointer address(const_reference x) const
    {
        return (x);
    }
    
    pointer allocate(size_type n, const_pointer cp = 0)
    {
        AllocatorBuffer *bufferAlloc = GetAllocatorBuffer();
        buffer->SetPointer((uint8*)bufferAlloc->GetPointer(n));
        return (pointer)(buffer->GetPointer());
    }
    
    void deallocate(pointer p, size_type n)
    {
    }
    
    void construct(pointer p, const value_type &x)
    {
        new (p)value_type(x);
    }
    
    void destroy(pointer p)
    {
        p->~value_type();
    }
    
private:
    void operator =(const FixedAllocator &);
    RefPtr<Pointer> buffer;
};

template<typename T>
inline bool operator ==(const FixedAllocator<T> &,
                        const FixedAllocator<T> &)
{
    return (false);
}

template<typename T>
inline bool operator !=(const FixedAllocator<T> &,
                        const FixedAllocator<T> &)
{
    return (true);
}

typedef std::basic_string<char, std::char_traits<char>, FixedAllocator<char> > FixedString;
    
}

#endif
#endif
