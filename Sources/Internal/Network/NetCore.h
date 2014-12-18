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

#ifndef __DAVAENGINE_NETCORE_H__
#define __DAVAENGINE_NETCORE_H__

#include <Base/BaseTypes.h>
#include <Base/Singleton.h>

#include <Network/Base/IOLoop.h>
#include <Network/INetDriver.h>
#include <Network/ServiceRegistrar.h>

namespace DAVA
{
namespace Net
{

class NetConfig;
class NetCore : public Singleton<NetCore>
{
public:
    typedef intptr_t TrackId;
    static const TrackId INVALID_TRACK_ID = 0;

public:
    NetCore();
    ~NetCore();

    IOLoop* Loop() { return &loop; }

    bool RegisterService(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter);

    TrackId CreateDriver(const NetConfig& config);
    bool DestroyDriver(TrackId id);

    int32 Run();
    int32 Poll();
    void Finish(bool withWait = false);

private:
    INetDriver* GetTrackedObject(TrackId id) const;
    void TrackedObjectStopped(INetDriver* obj);

    TrackId ObjectToTrackId(const INetDriver* obj) const;
    INetDriver* TrackIdToObject(TrackId id) const;

private:
    IOLoop loop;
    Set<INetDriver*> trackedObjects;
    ServiceRegistrar registrar;
};

//////////////////////////////////////////////////////////////////////////
inline NetCore::TrackId NetCore::ObjectToTrackId(const INetDriver* obj) const
{
    return reinterpret_cast<TrackId>(obj);
}

inline INetDriver* NetCore::TrackIdToObject(TrackId id) const
{
    return reinterpret_cast<INetDriver*>(id);
}

}   // namespace Net
}   // namespace DAVA


#endif  // __DAVAENGINE_NETCORE_H__
