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


#if defined(ENABLE_CEF_WEBVIEW)

#include <cef/include/cef_app.h>
#include "UI/Private/CEFDavaResourceHandler.h"

#include "Base/TypeHolders.h"
#include "Concurrency/Thread.h"
#include "Platform/SystemTimer.h"
#include "UI/Private/CEFController.h"

namespace DAVA
{
RefPtr<class CEFControllerImpl> cefControllerGlobal;

//--------------------------------------------------------------------------------------------------
//  CEF controller private implementation
//--------------------------------------------------------------------------------------------------
class CEFControllerImpl : public RefCounter
{
public:
    CEFControllerImpl();
    ~CEFControllerImpl();

    void Update();
    void ForceUpdate();
    void SetUpdateRate(uint32 n);

public:
    uint64 updateDelta;
    uint64 lastUpdateTime = 0;
};

CEFControllerImpl::CEFControllerImpl()
{
    SetUpdateRate(30);
    bool result = false;

    try
    {
        CefSettings settings;
        settings.no_sandbox = 1;
        settings.windowless_rendering_enabled = 1;
        settings.log_severity = LOGSEVERITY_DISABLE;
        //settings.log_severity = LOGSEVERITY_VERBOSE;
        CefString(&settings.browser_subprocess_path).FromASCII("CEFHelperProcess.exe");

        // CefInitialize replaces thread name, so we need to save it and restore
        //String threadName = Thread::GetCurrentThreadName();
        result = CefInitialize(CefMainArgs(), settings, nullptr, nullptr);
        //Thread::SetCurrentThreadName(threadName);
    }
    catch (...)
    {
    }

    result |= CefRegisterSchemeHandlerFactory("dava", "", new CEFDavaResourceHandlerFactory());
    DVASSERT_MSG(result == true, "CEF cannot be initialized");
}

CEFControllerImpl::~CEFControllerImpl()
{
    ForceUpdate();
    CefShutdown();
}

void CEFControllerImpl::Update()
{
    uint64 now = SystemTimer::Instance()->AbsoluteMS();
    if (now - lastUpdateTime >= updateDelta)
    {
        ForceUpdate();
        lastUpdateTime = now;
    }
}

void CEFControllerImpl::ForceUpdate()
{
    CefDoMessageLoopWork();
}

void CEFControllerImpl::SetUpdateRate(uint32 n)
{
    updateDelta = 1000 / n;
}

//--------------------------------------------------------------------------------------------------
//  CEF controller public implementation
//--------------------------------------------------------------------------------------------------
CEFController::CEFController()
{
    if (!cefControllerGlobal)
    {
        cefControllerGlobal.Set(new CEFControllerImpl);
    }
    cefControllerImpl = cefControllerGlobal;
}

CEFController::~CEFController()
{
    cefControllerImpl->ForceUpdate();
}

void CEFController::Update()
{
    cefControllerImpl->Update();
}

void CEFController::SetUpdateRate(uint32 n)
{
    cefControllerImpl->SetUpdateRate(n);
}

} // namespace DAVA

#endif // ENABLE_CEF_WEBVIEW
