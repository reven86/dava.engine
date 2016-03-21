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

#include "Base/Platform.h"
#include "Concurrency/Thread.h"
#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"
#include "Network/NetCore.h"
#include "Platform/SystemTimer.h"
#if defined(__DAVAENGINE_MACOS__)
    #include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#elif defined(__DAVAENGINE_WIN32__)
    #include "Platform/TemplateWin32/CorePlatformWin32.h"
#endif //PLATFORMS

#include "AssetCacheClient.h"

void FrameworkDidLaunched()
{
}

void FrameworkWillTerminate()
{
}

void CreateDAVA()
{
#if defined(__DAVAENGINE_MACOS__)
    DAVA::Core* core = new DAVA::CoreMacOSPlatform();
#elif defined(__DAVAENGINE_WIN32__)
    DAVA::Core* core = new DAVA::CoreWin32Platform();
#else // PLATFORMS
    static_assert(false, "Need create Core object");
#endif //PLATFORMS

    new DAVA::Logger();
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);
    DAVA::Logger::Instance()->EnableConsoleMode();

    new DAVA::FileSystem();
    DAVA::FilePath::InitializeBundleName();

    DAVA::FileSystem::Instance()->SetDefaultDocumentsDirectory();
    DAVA::FileSystem::Instance()->CreateDirectory(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory(), true);

    new DAVA::SystemTimer();

    DAVA::Thread::InitMainThread();

    new DAVA::Net::NetCore();
}

void ReleaseDAVA()
{
    DAVA::Net::NetCore::Instance()->Finish(true);
    DAVA::Net::NetCore::Instance()->Release();

    DAVA::SystemTimer::Instance()->Release();

    DAVA::FileSystem::Instance()->Release();
    DAVA::Logger::Instance()->Release();

    DAVA::Core::Instance()->Release();
}

int main(int argc, char* argv[])
{
    CreateDAVA();

    AssetCacheClient cacheClient;
    bool parsed = cacheClient.ParseCommandLine(argc, argv);
    if (parsed)
    {
        cacheClient.Process();
    }

    ReleaseDAVA();
    return cacheClient.GetExitCode();
}
