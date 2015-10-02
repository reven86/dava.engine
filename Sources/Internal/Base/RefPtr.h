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


#ifndef __DAVA_REF_PTR_H__
#define __DAVA_REF_PTR_H__

#include "Base/BaseObject.h"

namespace DAVA
{

//------------------------------------------------------------------------------
//Intrusive smartpointer for BaseObject and refcounted classes
//------------------------------------------------------------------------------
template <class T>
class RefPtr
{
public:
    RefPtr(T* ptr = nullptr);
    RefPtr(const RefPtr& ptr);
    RefPtr(RefPtr&& ptr);

    template <class Other>
    RefPtr(const RefPtr<Other>& ptr);
    template <class Other>
    RefPtr(RefPtr<Other>&& ptr);

    ~RefPtr();

    RefPtr& operator=(const RefPtr& ptr);
    RefPtr& operator=(RefPtr&& ptr);

    template <class Other>
    RefPtr& operator=(const RefPtr<Other>& ptr);
    template <class Other>
    RefPtr& operator=(RefPtr<Other>&& ptr);

    T* Get() const;
    void Set(T* ptr);
    void Reset();
    
    bool Valid() const;
    void Swap(RefPtr& ptr);
    
    T& operator*() const;
    T* operator->() const;
    
    operator bool();
    friend bool operator<(const RefPtr& lhs, const RefPtr& rhs);

    friend bool operator==(std::nullptr_t, const RefPtr& rhs) { return rhs.pointer == nullptr; }
    friend bool operator==(const RefPtr& lhs, std::nullptr_t) { return lhs.pointer == nullptr; }
    friend bool operator!=(std::nullptr_t, const RefPtr& rhs) { return rhs.pointer != nullptr; }
    friend bool operator!=(const RefPtr& lhs, std::nullptr_t) { return lhs.pointer != nullptr; }
    
    friend bool operator==(const T* lhs, const RefPtr& rhs) { return lhs == rhs.pointer; }
    friend bool operator==(const RefPtr& lhs, const T* rhs) { return lhs.pointer == rhs; }
    friend bool operator!=(const T* lhs, const RefPtr& rhs) { return !(lhs == rhs); }
    friend bool operator!=(const RefPtr& lhs, const T* rhs) { return !(lhs == rhs); }

    friend bool operator==(const RefPtr& lhs, const RefPtr& rhs) 
    { 
        return lhs.pointer == rhs.pointer; 
    }
    friend bool operator!=(const RefPtr& lhs, const RefPtr& rhs) { return !(lhs == rhs); }

private:
    template <class Other>
    friend class RefPtr;

    template <class U>
    RefPtr& Assign(U ptr);

    T* pointer;
};

//------------------------------------------------------------------------------
//Realization
//------------------------------------------------------------------------------
template <class T>
RefPtr<T>::RefPtr(T* ptr) : pointer(ptr) {}

template <class T>
RefPtr<T>::RefPtr(const RefPtr& ptr) : pointer(ptr.pointer)
{
    SafeRetain(pointer);
}

template <class T>
RefPtr<T>::RefPtr(RefPtr&& ptr) : RefPtr(ptr.pointer)
{
    ptr.pointer = nullptr;
}

template <class T>
template <class Other>
RefPtr<T>::RefPtr(const RefPtr<Other>& ptr) : RefPtr(ptr.pointer)
{
    SafeRetain(pointer);
}

template <class T>
template <class Other>
RefPtr<T>::RefPtr(RefPtr<Other>&& ptr) : RefPtr(ptr.pointer)
{
    ptr.pointer = nullptr;
}

template <class T>
RefPtr<T>::~RefPtr()
{
    SafeRelease(pointer);
}

template <class T>
RefPtr<T>& RefPtr<T>::operator=(const RefPtr& ptr)
{
    return Assign(ptr);
}

template <class T>
RefPtr<T>& RefPtr<T>::operator=(RefPtr&& ptr)
{
    return Assign(ptr);
}

template <class T>
template <class Other>
RefPtr<T>& RefPtr<T>::operator=(const RefPtr<Other>& ptr)
{
    return Assign(ptr);
}

template <class T>
template <class Other>
RefPtr<T>& RefPtr<T>::operator=(RefPtr<Other>&& ptr)
{
    return Assign(ptr);
}

template <class T>
T* RefPtr<T>::Get() const
{
    return pointer;
}

template <class T>
void RefPtr<T>::Set(T* ptr)
{
    Assign(ptr);
}

template <class T>
void RefPtr<T>::Reset()
{
    Assign(RefPtr<T>());
}

template <class T>
bool RefPtr<T>::Valid() const
{
    return pointer != nullptr;
}

template <class T>
void RefPtr<T>::Swap(RefPtr& ptr)
{
    std::swap(pointer, ptr.pointer);
}

template <class T>
T& RefPtr<T>::operator*() const
{
    return *pointer;
}

template <class T>
T* RefPtr<T>::operator->() const
{
    return pointer;
}

template <class T>
RefPtr<T>::operator bool()
{
    return pointer != nullptr;
}

template <class T>
bool operator<(const RefPtr<T>& lhs, const RefPtr<T>& rhs)
{
    return lhs.pointer < rhs.pointer;
}

template <class T>
template <class U>
RefPtr<T>& RefPtr<T>::Assign(U ptr)
{
    RefPtr<T> another(ptr);
    Swap(another);
    return *this;
}

template <typename T, typename ... Args>
RefPtr<T> MakeRefPtr(Args&&... args)
{
    return RefPtr<T>(new T(std::forward<Args>(args)...));
}

} // namespace DAVA

namespace std
{

template <class T>
void swap(DAVA::RefPtr<T>& lhs, DAVA::RefPtr<T>& rhs)
{
    lhs.Swap(rhs);
}

} // namespace std

#endif // __DAVA_REF_PTR_H__
