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


#include <algorithm>

#include <Debug/DVAssert.h>

#include <Network/ServiceRegistrar.h>

namespace DAVA
{
namespace Net
{

bool ServiceRegistrar::Register(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter, const char8* name)
{
    DVASSERT(creator != nullptr && deleter != nullptr);
    DVASSERT(!IsRegistered(serviceId));

    // Duplicate services are not allowed in registrar
    if (!IsRegistered(serviceId))
    {
        // If name hasn'y been set then generate name string based on service ID
        Array<char8, Entry::MAX_NAME_LENGTH> generatedName;
        if (NULL == name)
        {
            Snprintf(generatedName.data(), generatedName.size(), "service-%u", serviceId);
            name = generatedName.data();
        }
        Entry en(serviceId, name, creator, deleter);
        registrar.push_back(en);
        return true;
    }
    return false;
}

void ServiceRegistrar::UnregisterAll()
{
    registrar.clear();
}

IChannelListener* ServiceRegistrar::Create(uint32 serviceId, void* context) const
{
    const Entry* entry = FindEntry(serviceId);
    return entry != NULL ? entry->creator(serviceId, context)
                         : NULL;
}

bool ServiceRegistrar::Delete(uint32 serviceId, IChannelListener* obj, void* context) const
{
    const Entry* entry = FindEntry(serviceId);
    if (entry != NULL)
    {
        entry->deleter(obj, context);
        return true;
    }
    return false;
}

const char8* ServiceRegistrar::Name(uint32 serviceId) const
{
    const Entry* entry = FindEntry(serviceId);
    return entry != NULL ? entry->name
                         : NULL;
}

const ServiceRegistrar::Entry* ServiceRegistrar::FindEntry(uint32 serviceId) const
{
    Vector<Entry>::const_iterator i = std::find(registrar.begin(), registrar.end(), serviceId);
    return i != registrar.end() ? &*i : NULL;
}

}   // namespace Net
}   // namespace DAVA
