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


#ifndef __DAVAENGINE_SERVICEREGISTRAR_H__
#define __DAVAENGINE_SERVICEREGISTRAR_H__

#include <Base/BaseTypes.h>
#include <Functional/Function.h>

namespace DAVA
{
namespace Net
{

struct IChannelListener;

using ServiceCreator = Function<IChannelListener* (uint32 serviceId, void* context)>;
using ServiceDeleter = Function<void(IChannelListener* obj, void* context)>;

class ServiceRegistrar
{
private:
    struct Entry
    {
        static const size_t MAX_NAME_LENGTH = 32;

        Entry(uint32 id, const char8* serviceName, ServiceCreator creatorFunc, ServiceDeleter deleterFunc);

        uint32 serviceId;
        char8 name[MAX_NAME_LENGTH];
        ServiceCreator creator;
        ServiceDeleter deleter;
    };

    friend bool operator == (const Entry& entry, uint32 serviceId);

public:
    bool Register(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter, const char8* name = NULL);
    void UnregisterAll();
    bool IsRegistered(uint32 serviceId) const;

    IChannelListener* Create(uint32 serviceId, void* context) const;
    bool Delete(uint32 serviceId, IChannelListener* obj, void* context) const;

    const char8* Name(uint32 serviceId) const;

private:
    const Entry* FindEntry(uint32 serviceId) const;

private:
    Vector<Entry> registrar;
};

//////////////////////////////////////////////////////////////////////////
inline ServiceRegistrar::Entry::Entry(uint32 id, const char8* serviceName, ServiceCreator creatorFunc, ServiceDeleter deleterFunc)
    : serviceId(id)
    , creator(creatorFunc)
    , deleter(deleterFunc)
{
#if defined(__DAVAENGINE_WINDOWS__)
    strncpy_s(name, serviceName, _TRUNCATE);
#else
    strncpy(name, serviceName, MAX_NAME_LENGTH);
    name[MAX_NAME_LENGTH - 1] = '\0';
#endif
}

inline bool ServiceRegistrar::IsRegistered(uint32 serviceId) const
{
    return std::find(registrar.begin(), registrar.end(), serviceId) != registrar.end();
}

inline bool operator == (const ServiceRegistrar::Entry& entry, uint32 serviceId)
{
    return entry.serviceId == serviceId;
}

}   // namespace Net
}   // namespace DAVA

#endif  // __DAVAENGINE_SERVICEREGISTRAR_H__
