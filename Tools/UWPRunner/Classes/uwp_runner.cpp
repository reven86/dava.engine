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
#include "FileSystem/FileSystem.h"
#include "Network/Services/LogConsumer.h"
#include "Network/SimpleNetworking/SimpleNetCore.h"
#include "Network/SimpleNetworking/SimpleNetService.h"

#include <Objbase.h>
#include "runner.h"
#include "uwp_runner.h"

using namespace DAVA;

const Net::SimpleNetService* gNetLogger = nullptr;

void Run(Runner& runner);
void Start(Runner& runner);

bool InitializeNetwork();
void WaitApp();

void FrameworkDidLaunched()
{
    //Parse arguments
    PackageOptions commandLineOptions = ParseCommandLine();
    if (!CheckOptions(commandLineOptions))
    {
        return;
    }

    //Extract manifest from package
    FilePath manifest = ExtractManifest(commandLineOptions.package.Get());
    SCOPE_EXIT
    {
        if (manifest.Exists())
            FileSystem::Instance()->DeleteFile(manifest);
    };
    
    //Convert profile to qt winrt runner profile
    QString profile = commandLineOptions.profile == "local" ? QStringLiteral("appx") 
                                                            : QStringLiteral("appxphone");
    //Create Qt runner
    Runner runner(QString::fromStdString(commandLineOptions.package.Get()),
                  QString::fromStdString(manifest.GetAbsolutePathname()),
                  QString::fromStdString(commandLineOptions.dependencies.Get()),
                  QStringList(), 
                  profile);

    //Check runner state
    if (!runner.isValid())
    {
        DVASSERT_MSG(false, "Runner core is not valid");
        return;
    }

    Run(runner);
}

void Run(Runner& runner)
{
    //Init network
    if (!InitializeNetwork())
    {
        DVASSERT_MSG(false, "Unable to initialize network");
        return;
    }

    //Start app
    Start(runner);

    //Wait app exit
    WaitApp();

    //remove app package after working
    if (!runner.remove())
    {
        DVASSERT_MSG(false, "Unable to remove package");
        return;
    }
}

void Start(Runner& runner)
{
    Net::SimpleNetCore netcore;

    if (!runner.install(true))
    {
        DVASSERT_MSG(false, "Can't install application package");
        return;
    }

    if (!runner.start())
    {
        DVASSERT_MSG(false, "Can't start application");
        return;
    }
}

bool InitializeNetwork()
{
    Net::SimpleNetCore* netcore = new Net::SimpleNetCore;
    Net::Endpoint endPoint("127.0.0.1", 7777);

    Net::LogConsumer::Options options;
    options.rawOutput = true;
    options.writeToConsole = true;
    auto logConsumer = std::make_unique<Net::LogConsumer>(std::cref(options));

    gNetLogger = netcore->RegisterService(std::move(logConsumer), 
        Net::IConnectionManager::kServerRole, endPoint, "LogConsumer");

    return gNetLogger != nullptr;
}

void WaitApp()
{
    while (true)
    {
        Thread::Sleep(1000);
        if (!gNetLogger->IsActive())
        {
            break;
        }
    }
}

void FrameworkWillTerminate()
{
    //cleanup network
    Net::SimpleNetCore::Instance()->Release();
}