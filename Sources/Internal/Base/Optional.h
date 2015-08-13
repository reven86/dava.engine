/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_OPTIONAL_H__
#define __DAVAENGINE_OPTIONAL_H__

#include <cassert>
#include <type_traits>

namespace DAVA
{

//-------------------------------------------------------------------------------------------------
struct EmptyOptional {};

template <typename T>
class Optional
{
    friend Optional;
public:
    Optional() = default;
    Optional(EmptyOptional);

    Optional(const Optional& other);
    Optional(Optional&& other);
    Optional(const T& value);
    Optional(T&& value);

    ~Optional();

    bool IsSet() const;

    void Set(const Optional& other);
    void Set(Optional&& other);
    void Set(const T& value);
    void Set(T&& value);
    void Reset();

    T& Get();
    const T& Get() const;

    Optional& operator=(const Optional& other);
    Optional& operator=(Optional&& other);
    Optional& operator=(const T& value);
    Optional& operator=(T&& value);

private:
    using aligned_storage = 
        typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;

    aligned_storage storage;
    bool isSet = false;
};

//-------------------------------------------------------------------------------------------------
//Implementation
//-------------------------------------------------------------------------------------------------
template <typename T>
Optional<T>::Optional(EmptyOptional) : Optional() {}

template <typename T>
Optional<T>::Optional(const Optional& other) : Optional()
{
    if (other.IsSet())
    {
        Set(other.Get());
    }
}

template <typename T>
Optional<T>::Optional(Optional&& other) : Optional()
{
    if (other.IsSet())
    {
        Set(std::forward<Optional>(other.Get()));
    }
}

template <typename T>
Optional<T>::Optional(const T& value)
{
    Set(value);
}

template <typename T>
Optional<T>::Optional(T&& value)
{
    Set(std::forward<T>(value));
}

template <typename T>
Optional<T>::~Optional()
{
    Reset();
}

template <typename T>
bool Optional<T>::IsSet() const
{
    return isSet;
}

template <typename T>
void Optional<T>::Set(const Optional& other)
{
    Reset();

    if (other.IsSet())
    {
        Set(other.Get());
    }
}

template <typename T>
void Optional<T>::Set(Optional&& other)
{
    Reset();

    if (other.IsSet())
    {
        Set(std::move(other.Get()));
    }
}

template <typename T>
void Optional<T>::Set(const T& value)
{
    Reset();

    new (&storage) T(value);
    isSet = true;
}

template <typename T>
void Optional<T>::Set(T&& value)
{
    Reset();

    new (&storage) T(std::forward<T>(value));
    isSet = true;
}

template <typename T>
void Optional<T>::Reset()
{
    if (IsSet())
    {
        Get().~T();
        isSet = false;
    }
}

template <typename T>
T& Optional<T>::Get()
{
    assert(isSet);
    return *reinterpret_cast<T*>(&storage);
}

template <typename T>
const T& Optional<T>::Get() const
{
    assert(isSet);
    return *reinterpret_cast<const T*>(&storage);
}

template <typename T>
Optional<T>& Optional<T>::operator=(const Optional& other)
{
    Set(other);
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(Optional&& other)
{
    Set(other);
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(const T& value)
{
    Set(value);
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(T&& value)
{
    Set(std::forward<T>(value));
    return *this;
}

template <typename T>
bool operator==(const Optional<T>& rhs, const Optional<T>& lhs)
{
    if (!rhs.IsSet() || !lhs.IsSet())
        return rhs.IsSet() == lhs.IsSet();
    return rhs.Get() == lhs.Get();
}

template <typename T, typename U>
bool operator==(const Optional<T>& rhs, const U& lhs)
{
    if (!rhs.IsSet())
        return false;
    return rhs.Get() == lhs;
}

template <typename T, typename U>
bool operator==(const T& rhs, const Optional<U>& lhs)
{
    if (!lhs.IsSet())
        return false;
    return rhs == lhs.Get();
}

template <typename T>
bool operator!=(const Optional<T>& rhs, const Optional<T>& lhs)
{
    return !(rhs == lhs);
}

template <typename T, typename U>
bool operator!=(const Optional<T>& rhs, const U& lhs)
{
    return !(rhs == lhs);
}

template <typename T, typename U>
bool operator!=(const T& rhs, const Optional<U>& lhs)
{
    return !(rhs == lhs);
}

}

#endif  // __DAVAENGINE_OPTIONAL_H__