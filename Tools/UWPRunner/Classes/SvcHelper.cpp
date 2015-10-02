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


#include "Concurrency/Thread.h"
#include "Debug/DVAssert.h"
#include "SvcHelper.h"

namespace DAVA
{

SvcHelper::SvcHelper(const String &name)
    : serviceName(name)
{
    serviceControlManager = ::OpenSCManager(0, 0, 0);
    if (!serviceControlManager)
    {
        DVASSERT_MSG(false, "Can't open Service Control Manager");
        return;
    }

    service = ::OpenService(serviceControlManager, name.c_str(), SC_MANAGER_ALL_ACCESS);
    if (!service)
    {
        DVASSERT_MSG(false, "Can't open service " + name);
    }
}

SvcHelper::~SvcHelper()
{
    if (service)
        ::CloseServiceHandle(service);

    if (serviceControlManager)
        ::CloseServiceHandle(serviceControlManager);
}

String SvcHelper::ServiceName() const
{
    return serviceName;
}

String SvcHelper::ServiceDescription() const
{
    Array<char, 8 * 1024> data;
    DWORD bytesNeeded;

    BOOL res = ::QueryServiceConfig2(service, 
        SERVICE_CONFIG_DESCRIPTION, (LPBYTE)data.data(), data.size(), &bytesNeeded);

    if (!res)
    {
        DVASSERT_MSG(false, "Can't query service description");
        return "";
    }

    return String(data.data());
}

bool SvcHelper::IsInstalled() const
{
    return service != nullptr;
}

bool SvcHelper::IsRunning() const
{
    SERVICE_STATUS info;
    if (!::QueryServiceStatus(service, &info))
    {
        DVASSERT_MSG(false, "Can't query service status");
        return false;
    }
    
    return info.dwCurrentState != SERVICE_STOPPED;
}

bool SvcHelper::Start()
{
    return ::StartService(service, 0, nullptr) == TRUE;
}

bool SvcHelper::Stop()
{
    SERVICE_STATUS status;
    if (::ControlService(service, SERVICE_CONTROL_STOP, &status) == TRUE) {
        bool stopped = status.dwCurrentState == SERVICE_STOPPED;
        unsigned i = 0;

        while (!stopped && i < 10) {
            Thread::Sleep(200);
            if (!::QueryServiceStatus(service, &status))
                break;

            stopped = status.dwCurrentState == SERVICE_STOPPED;
            ++i;
        }

        return stopped;
    }
    
    return false;
}

}  // namespace DAVA