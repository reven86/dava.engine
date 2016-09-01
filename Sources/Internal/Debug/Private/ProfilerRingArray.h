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
    RingArray(RingArray&& a)
    {
        elements = std::move(a.elements);
        mask = a.mask;
        head = a.head.load();
    }
    RingArray& operator=(const RingArray& a)
    {
        if (this != &a)
            new (this) RingArray(a);
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
        return reverse_iterator(elements.data(), ((head - 1) & mask) | (mask + 1), mask);
    }
    inline reverse_iterator rend()
    {
        return reverse_iterator(elements.data(), (head - 1) & mask, mask);
    }
    inline size_t size() const
    {
        return _Size;
    }

protected:
    class base_iterator
    {
    public:
        base_iterator() = default;
        ~base_iterator() = default;

        inline bool operator==(const base_iterator& it) const
        {
            return index == it.index;
        }
        inline bool operator!=(const base_iterator& it) const
        {
            return index != it.index;
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
            arrayData = it.arrayData;
            mask = it.mask;
            index = it.index;
        }
        inline operator reverse_iterator() const
        {
            return reverse_iterator(arrayData, index, mask);
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
            ++index;
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
            --index;
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

        friend class RingArray;
    };

    class reverse_iterator : public base_iterator
    {
    public:
        reverse_iterator(const iterator& it)
        {
            arrayData = it.arrayData;
            mask = it.mask;
            index = it.index;
        }
        inline operator iterator() const
        {
            return iterator(arrayData, index, mask);
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
            --index;
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
            ++index;
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

        friend class RingArray;
    };

protected:
    std::array<T, _Size> elements;
    uint32 mask = _Size - 1;
    std::atomic<uint32> head = { 0 };
};

}; //ns