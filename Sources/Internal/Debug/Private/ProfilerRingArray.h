#pragma once

#include "Base/BaseTypes.h"
#include "Math/MathHelpers.h"
#include "Debug/DVAssert.h"
#include <atomic>

namespace DAVA
{
//////////////////////////////////////////////////////////////////////////
//'pseudo-thread-safe' ring array. It's provide only thread-safe obtaining
// of reference to element. So, don't write to array in parallel with
// array iterating.
// It's made for performance reasons.
//////////////////////////////////////////////////////////////////////////

template <class T>
class ProfilerRingArray
{
public:
    ProfilerRingArray(uint32 _size)
    {
        DVASSERT(IsPowerOf2(_size) && "Size of RingArray should be pow of two");
        elementsCount = _size;
        mask = elementsCount - 1;
        elements = new T[elementsCount];
    }
    ~ProfilerRingArray()
    {
        SafeDeleteArray(elements);
    }
    ProfilerRingArray(const ProfilerRingArray& a)
    {
        elementsCount = a.elementsCount;
        elements = new T[elementsCount];
        memcpy(elements, a.elements, elementsCount * sizeof(T));
        mask = a.mask;
        head = a.head.load();
    }
    ProfilerRingArray& operator=(const ProfilerRingArray& a)
    {
        if (this != &a)
        {
            SafeDeleteArray(elements);
            new (this) ProfilerRingArray(a);
        }
        return (*this);
    }

    class iterator;
    class reverse_iterator;

    inline T& next()
    {
        return elements[(head++) & mask];
    }
    inline iterator begin()
    {
        return iterator(elements, head & mask, mask);
    }
    inline iterator end()
    {
        return iterator(elements, (head & mask) | (mask + 1), mask);
    }
    inline reverse_iterator rbegin()
    {
        return reverse_iterator(elements, ((head - 1) & mask) | (mask + 1), mask);
    }
    inline reverse_iterator rend()
    {
        return reverse_iterator(elements, (head - 1) & mask, mask);
    }
    inline size_t size() const
    {
        return elementsCount;
    }

protected:
    class base_iterator
    {
    public:
        base_iterator() = default;
        ~base_iterator() = default;

        inline bool operator==(const base_iterator& it) const
        {
            return (index == it.index) && (arrayData == it.arrayData);
        }
        inline bool operator!=(const base_iterator& it) const
        {
            return !(*this == it);
        }
        inline T& operator*() const
        {
            return arrayData[index & mask];
        }
        inline T* operator->() const
        {
            return &arrayData[index & mask];
        }

        base_iterator(T* data, uint32 _index, uint32 _mask)
            : arrayData(data)
            , index(_index)
            , mask(_mask)
        {
        }

        T* arrayData = nullptr;
        uint32 index = 0;
        uint32 mask = 0;
    };

public:
    class iterator : public base_iterator
    {
    public:
        iterator(const reverse_iterator& it)
        {
            this->arrayData = it.arrayData;
            this->mask = it.mask;
            this->index = it.index;
        }
        inline operator reverse_iterator() const
        {
            return reverse_iterator(this->arrayData, this->index, this->mask);
        }
        inline iterator operator+(uint32 n) const
        {
            iterator it(*this);
            it.index += n;
            return it;
        }
        inline iterator operator-(uint32 n) const
        {
            iterator it(*this);
            it.index -= n;
            return it;
        }
        inline iterator& operator++()
        {
            ++this->index;
            return *this;
        }
        inline iterator operator++(int)
        {
            iterator prev = *this;
            ++(*this);
            return prev;
        }
        inline iterator& operator--()
        {
            --this->index;
            return *this;
        }
        inline iterator operator--(int)
        {
            iterator prev = *this;
            --(*this);
            return prev;
        }

    protected:
        iterator(T* data, uint32 _index, uint32 _mask)
            : base_iterator(data, _index, _mask)
        {
        }

        friend class ProfilerRingArray;
    };

    class reverse_iterator : public base_iterator
    {
    public:
        reverse_iterator(const iterator& it)
        {
            this->arrayData = it.arrayData;
            this->mask = it.mask;
            this->index = it.index;
        }
        inline operator iterator() const
        {
            return iterator(this->arrayData, this->index, this->mask);
        }
        inline reverse_iterator operator+(uint32 n) const
        {
            reverse_iterator it(*this);
            it.index -= n;
            return it;
        }
        inline reverse_iterator operator-(uint32 n) const
        {
            reverse_iterator it(*this);
            it.index += n;
            return it;
        }
        inline reverse_iterator& operator++()
        {
            --this->index;
            return *this;
        }
        inline reverse_iterator operator++(int)
        {
            reverse_iterator prev = *this;
            ++(*this);
            return prev;
        }
        inline reverse_iterator& operator--()
        {
            ++this->index;
            return *this;
        }
        inline reverse_iterator operator--(int)
        {
            reverse_iterator prev = *this;
            --(*this);
            return prev;
        }

    protected:
        reverse_iterator(T* data, uint32 _index, uint32 _mask)
            : base_iterator(data, _index, _mask)
        {
        }

        friend class ProfilerRingArray;
    };

protected:
    T* elements = nullptr;
    uint32 elementsCount = 0;
    uint32 mask = 0;
    std::atomic<uint32> head = { 0 };
};

}; //ns