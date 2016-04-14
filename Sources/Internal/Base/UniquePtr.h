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

#pragma once

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
template <typename BASE_OBJECT>
class UniquePtr
{
public:
    explicit UniquePtr(BASE_OBJECT* p);
    UniquePtr(const UniquePtr&) = delete;

    ~UniquePtr();

    const UniquePtr& operator=(const UniquePtr&) = delete;
    const UniquePtr& operator=(BASE_OBJECT* p);
    BASE_OBJECT& operator*() const;
    BASE_OBJECT* operator->() const;
    operator BASE_OBJECT*() const;
    explicit operator bool() const;
    operator void*() const;

    BASE_OBJECT* get() const;
    void reset(BASE_OBJECT* p = nullptr);

private:
    BASE_OBJECT* object;
};

template <typename BASE_OBJECT>
UniquePtr<BASE_OBJECT>::UniquePtr(BASE_OBJECT* p)
    : object(p)
{
    DVASSERT(p ? p->GetRetainCount() == 1 : true);
}

template <typename BASE_OBJECT>
const UniquePtr<BASE_OBJECT>& UniquePtr<BASE_OBJECT>::operator=(BASE_OBJECT* p)
{
    if (p != object)
    {
        SafeRelease(object);
        object = p;
        DVASSERT(p ? p->GetRetainCount() == 1 : true);
    }

    return *this;
}

template <typename BASE_OBJECT>
UniquePtr<BASE_OBJECT>::~UniquePtr()
{
    SafeRelease(object);
}

template <typename BASE_OBJECT>
BASE_OBJECT& UniquePtr<BASE_OBJECT>::operator*() const
{
    return *object;
}

template <typename BASE_OBJECT>
BASE_OBJECT* UniquePtr<BASE_OBJECT>::operator->() const
{
    return object;
}

template <typename BASE_OBJECT>
UniquePtr<BASE_OBJECT>::operator BASE_OBJECT*() const
{
    return object;
}

template <typename BASE_OBJECT>
BASE_OBJECT* UniquePtr<BASE_OBJECT>::get() const
{
    return object;
}

template <typename BASE_OBJECT>
void UniquePtr<BASE_OBJECT>::reset(BASE_OBJECT* p)
{
    if (p != object)
    {
        SafeRelease(object);
        object = p;
        DVASSERT(p ? p->GetRetainCount() == 1 : true);
    }
}

template <typename BASE_OBJECT>
UniquePtr<BASE_OBJECT>::operator bool() const
{
    return object != nullptr;
}

template <typename BASE_OBJECT>
UniquePtr<BASE_OBJECT>::operator void*() const
{
    return object;
}

} // namespace DAVA
