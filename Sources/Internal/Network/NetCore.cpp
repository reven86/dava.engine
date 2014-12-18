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

#include <Base/FunctionTraits.h>
#include <Debug/DVAssert.h>

#include <Network/NetDriver.h>
#include <Network/NetCore.h>

namespace DAVA
{
namespace Net
{

NetCore::NetCore() : loop(true)
{

}

NetCore::~NetCore()
{
    DVASSERT(true == trackedObjects.empty());
}

bool NetCore::RegisterService(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter)
{
    return registrar.Register(serviceId, creator, deleter);
}

NetCore::TrackId NetCore::CreateDriver(const NetConfig& config)
{
    NetDriver* driver = new NetDriver(&loop, registrar);
    if (true == driver->ApplyConfig(config))
    {
        trackedObjects.insert(driver);
        driver->Start();
        return ObjectToTrackId(driver);
    }
    delete driver;
    return INVALID_TRACK_ID;
}

int32 NetCore::Run()
{
    return loop.Run(IOLoop::RUN_DEFAULT);
}

int32 NetCore::Poll()
{
    return loop.Run(IOLoop::RUN_NOWAIT);
}

void NetCore::Finish(bool withWait)
{
    for (Set<INetDriver*>::iterator i = trackedObjects.begin(), e = trackedObjects.end();i != e;++i)
        (*i)->Stop(MakeFunction(this, &NetCore::TrackedObjectStopped));
    loop.PostQuit();
    if (withWait)
        loop.Run(IOLoop::RUN_DEFAULT);
}

INetDriver* NetCore::GetTrackedObject(TrackId id) const
{
    Set<INetDriver*>::const_iterator i = trackedObjects.find(TrackIdToObject(id));
    return i != trackedObjects.end() ? *i
                                     : NULL;
}

void NetCore::TrackedObjectStopped(INetDriver* obj)
{
    DVASSERT(obj != NULL && GetTrackedObject(ObjectToTrackId(obj)) != NULL);

    if (trackedObjects.find(obj) != trackedObjects.end())
    {
        trackedObjects.erase(obj);
        delete obj;
    }
}

}   // namespace Net
}   // namespace DAVA
