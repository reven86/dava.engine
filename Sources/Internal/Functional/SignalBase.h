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

#ifndef __DAVA_SIGNAL_BASE_H__
#define __DAVA_SIGNAL_BASE_H__

#include "Base/BaseTypes.h"

namespace DAVA {

using SigConnectionID = size_t;

class TrackedObject;
class SignalBase
{
public:
    virtual ~SignalBase() = default;
    virtual void Disconnect(TrackedObject*) = 0;

    static SigConnectionID GetUniqueConnectionID()
    {
        static Atomic<SigConnectionID> counter = { 0 };
        return ++counter;
    }
};

class TrackedObject
{
public:
    void Track(SignalBase *signal)
    {
        trackedSignals.insert(signal);
    }

    void Untrack(SignalBase *signal)
    {
        trackedSignals.erase(signal);
    }
    
    template<typename T>
    static TrackedObject* Cast(T *t)
    {
        return Detail<std::is_base_of<TrackedObject, T>::value>::Cast(t);
    }

protected:
    Set<SignalBase*> trackedSignals;

    template<bool is_derived_from_tracked_obj>
    struct Detail;

    ~TrackedObject()
    {
        while (trackedSignals.size() > 0)
        {
            auto it = trackedSignals.begin();
            (*it)->Disconnect(this);
        }
    }
};
    
template<>
struct TrackedObject::Detail<false>
{
    static TrackedObject* Cast(void* t) { return nullptr; }
};

template<>
struct TrackedObject::Detail<true>
{
    template<typename T>
    static TrackedObject* Cast(T* t) { return static_cast<TrackedObject *>(t); }
};

} // namespace DAVA

#endif // __DAVA_SIGNAL_BASE_H__
