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


#ifndef __DAVAENGINE_CONCURRENT_OBJECT_H__
#define __DAVAENGINE_CONCURRENT_OBJECT_H__

#include "Concurrency/Mutex.h"
#include "Concurrency/UniqueLock.h"

namespace DAVA
{
    
//------------------------------------------------------------------------------
//Class for concurrent use of objects. Stored object protected by mutex
//------------------------------------------------------------------------------
template <typename T, typename MutexType = Mutex>
class ConcurrentObject
{
    friend class Accessor;
public:
    template <typename... Args>
    ConcurrentObject(Args&&... args) : object(std::forward<Args>(args)...) {}

    class Accessor
    {
    public:
        Accessor(ConcurrentObject& object) 
            : objectRef(object.object), guard(object.mutex) {}

        Accessor(Accessor&& other) 
            : objectRef(other.objectRef), guard(std::move(other.guard)) {}

        T& operator*() { return objectRef; }
        T* operator->() { return &objectRef; }

    private:
        T& objectRef;
        UniqueLock<MutexType> guard;
    };

    Accessor GetAccessor() { return Accessor(*this); }

    T Load() { return *Accessor(*this); }
    void Store(const T& value) { *Accessor(*this) = value; }

private:
    T object;
    MutexType mutex;
};

}

#endif //  __DAVAENGINE_CONCURRENT_OBJECT_H__