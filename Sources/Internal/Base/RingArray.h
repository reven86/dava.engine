#pragma once

#include "Base/BaseTypes.h"
#include <atomic>

namespace DAVA
{
template <class T, uint32 _Size>
class RingArray
{
public:
    static_assert(((_Size - 1) & _Size) == 0 && _Size != 0, "Size of RingArray should be pow of two");

    RingArray() = default;
    RingArray(const RingArray& a)
    {
        memcpy(elements.data(), a.elements.data(), elements.size() * sizeof(T));
        mask = a.mask;
        head = a.head.load();
    }
    RingArray& operator=(const RingArray& a)
    {
        memcpy(elements.data(), a.elements.data(), elements.size() * sizeof(T));
        mask = a.mask;
        head = a.head.load();
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
        return iterator(elements.data(), head & mask, mask);
    }
    inline iterator end()
    {
        return iterator(elements.data(), (head & mask) | (mask + 1), mask);
    }
    inline reverse_iterator rbegin()
    {
        return reverse_iterator(elements.data(), (head - 1) & mask, mask);
    }
    inline reverse_iterator rend()
    {
        return reverse_iterator(elements.data(), ((head - 1) & mask) | (mask + 1), mask);
    }
    inline size_t size()
    {
        return _Size;
    }

    class iterator
    {
    public:
        iterator() = default;
        ~iterator() = default;

        iterator(const reverse_iterator& it)
        {
            arrayData = it.arrayData;
            exmask = it.exmask;
            index = it.index ^ ((exmask >> 1) + 1);
        }

        inline iterator operator+(uint32 n)
        {
            iterator it(*this);
            it.index = (index + n) & exmask;
            return it;
        }
        inline iterator operator-(uint32 n)
        {
            iterator it(*this);
            it.index = (index - n) & exmask;
            return it;
        }
        inline iterator& operator++()
        {
            index = (index + 1) & exmask;
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
            index = (index - 1) & exmask;
            return *this;
        }
        inline iterator operator--(int)
        {
            iterator prev = *this;
            --(*this);
            return prev;
        }
        inline bool operator==(const iterator& it)
        {
            return index == it.index;
        }
        inline bool operator!=(const iterator& it)
        {
            return index != it.index;
        }
        inline T& operator*()
        {
            return arrayData[index & (exmask >> 1)];
        }
        inline T* operator->()
        {
            return &arrayData[index & (exmask >> 1)];
        }
        inline operator reverse_iterator() const
        {
            return reverse_iterator(arrayData, index ^ ((exmask >> 1) + 1), exmask >> 1);
        }

    protected:
        iterator(T* data, uint32 _index, uint32 _mask)
            : arrayData(data)
            , index(_index)
        {
            exmask = (_mask << 1) + 1;
        }

        T* arrayData = nullptr;
        uint32 index = 0;
        uint32 exmask = 0;

        friend class RingArray;
        friend class reverse_iterator;
    };

    class reverse_iterator
    {
    public:
        reverse_iterator() = default;
        ~reverse_iterator() = default;

        reverse_iterator(const iterator& it)
        {
            arrayData = it.arrayData;
            exmask = it.exmask;
            index = it.index ^ ((exmask >> 1) + 1);
        }

        inline reverse_iterator operator+(uint32 n)
        {
            reverse_iterator it(*this);
            it.index = (index - n) & exmask;
            return it;
        }
        inline reverse_iterator operator-(uint32 n)
        {
            reverse_iterator it(*this);
            it.index = (index + n) & exmask;
            return it;
        }
        inline reverse_iterator& operator++()
        {
            index = (index - 1) & exmask;
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
            index = (index + 1) & exmask;
            return *this;
        }
        inline reverse_iterator operator--(int)
        {
            reverse_iterator prev = *this;
            --(*this);
            return prev;
        }
        inline bool operator==(const reverse_iterator& it)
        {
            return index == it.index;
        }
        inline bool operator!=(const reverse_iterator& it)
        {
            return index != it.index;
        }
        inline T& operator*()
        {
            return arrayData[index & (exmask >> 1)];
        }
        inline T* operator->()
        {
            return &arrayData[index & (exmask >> 1)];
        }
        inline operator iterator() const
        {
            return iterator(arrayData, index ^ ((exmask >> 1) + 1), exmask >> 1);
        }

    protected:
        reverse_iterator(T* data, uint32 _index, uint32 _mask)
            : arrayData(data)
            , index(_index)
        {
            exmask = (_mask << 1) + 1;
        }

        T* arrayData = nullptr;
        uint32 index = 0;
        uint32 exmask = 0;

        friend class RingArray;
        friend class iterator;
    };

protected:
    std::array<T, _Size> elements;
    uint32 mask = _Size - 1;
    std::atomic<uint32> head = { 0 };
};

}; //ns