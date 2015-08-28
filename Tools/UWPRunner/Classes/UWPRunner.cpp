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


#include "Concurrency/LockGuard.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/Thread.h"
#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Network/Services/LogConsumer.h"
#include "Network/SimpleNetworking/SimpleNetCore.h"
#include "Network/SimpleNetworking/SimpleNetService.h"

#include <Objbase.h>
#include "SvcHelper.h"
#include "runner.h"
#include "RegKey.h"
#include "UWPRunner.h"

using namespace DAVA;

const Net::SimpleNetService* gNetLogger = nullptr;

void Run(Runner& runner);
void Start(Runner& runner);

bool InitializeNetwork(bool isMobileDevice);
bool ConfigureIpOverUsb();
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
    DVASSERT_MSG_RET(runner.isValid(), "Runner core is not valid");
    Run(runner);
}

void Run(Runner& runner)
{
    //figure out if app should be started on mobile device
    bool isMobileDevice = runner.profile() == QStringLiteral("appxphone");

    //Init network
    DVASSERT_MSG_RET(InitializeNetwork(isMobileDevice), "Unable to initialize network");

    //Start app
    Start(runner);

    //Wait app exit
    WaitApp();

    //remove app package after working
    DVASSERT_MSG_RET(runner.remove(), "Unable to remove package");
}

void Start(Runner& runner)
{
    DVASSERT_MSG_RET(runner.install(true), "Can't install application package");
    DVASSERT_MSG_RET(runner.start(), "Can't install application package");
}

bool InitializeNetwork(bool isMobileDevice)
{
    if (isMobileDevice)
    {
        DVASSERT_MSG_RET(ConfigureIpOverUsb(), "Can't configure IpOverUsb service", false);
    }

    uint16 port;
    Net::IConnectionManager::ConnectionRole role;

    if (isMobileDevice)
    {
        port = Net::SimpleNetCore::UWPRemotePort;
        role = Net::IConnectionManager::ClientRole;
    }
    else
    {
        port = Net::SimpleNetCore::UWPLocalPort;
        role = Net::IConnectionManager::ServerRole;
    }
    Net::Endpoint endPoint("127.0.0.1", port);

    Net::LogConsumer::Options options;
    options.rawOutput = true;
    options.writeToConsole = true;
    auto logConsumer = std::make_unique<Net::LogConsumer>(std::cref(options));

    Net::SimpleNetCore* netcore = new Net::SimpleNetCore;
    gNetLogger = netcore->RegisterService(
        std::move(logConsumer), role, endPoint, "RawLogConsumer", isMobileDevice);

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

Optional<bool> UpdateIpOverUsbConfig(RegKey& key)
{
    const String desiredDestAddr = "127.0.0.1";
    const DWORD  desiredDestPort = Net::SimpleNetCore::UWPRemotePort;
    const String desiredLocalAddr = desiredDestAddr;
    const DWORD  desiredLocalPort = desiredDestPort;
    bool changed = false;

    Optional<String> address = key.QueryString("DestinationAddress");
    if (address != desiredDestAddr)
    {
        DVASSERT_MSG_RET(key.SetValue("DestinationAddress", desiredDestAddr),
                         "Unable to set DestinationAddress", EmptyOptional());
        changed |= true;
    }

    Optional<DWORD> port = key.QueryDWORD("DestinationPort");
    if (port != desiredDestPort)
    {
        DVASSERT_MSG_RET(key.SetValue("DestinationPort", desiredDestPort),
                         "Unable to set DestinationPort", EmptyOptional());
        changed |= true;
    }

    address = key.QueryString("LocalAddress");
    if (address != desiredLocalAddr)
    {
        DVASSERT_MSG_RET(key.SetValue("LocalAddress", desiredLocalAddr),
                         "Unable to set LocalAddress", EmptyOptional());
        changed |= true;
    }

    port = key.QueryDWORD("LocalPort");
    if (port != desiredLocalPort)
    {
        DVASSERT_MSG_RET(key.SetValue("LocalPort", desiredLocalPort),
                         "Unable to set LocalPort", EmptyOptional());
        changed |= true;
    }

    return changed;
}

bool RestartIpOverUsb()
{
    //open service
    SvcHelper service("IpOverUsbSvc");
    DVASSERT_MSG_RET(service.IsInstalled(), "Can't open IpOverUsb service", false);

    //stop it
    DVASSERT_MSG_RET(service.Stop(), "Can't stop IpOverUsb service", false);
    
    //start it
    DVASSERT_MSG_RET(service.Start(), "Can't start IpOverUsb service", false);

    //waiting for service starting
    Thread::Sleep(1000);

    return true;
}

bool ConfigureIpOverUsb()
{
    bool needRestart = false;

    //open or create key
    RegKey key(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\IpOverUsb\\DavaDebugging", true);
    DVASSERT_MSG_RET(key.IsExist(), "Can't open or create key", false);
    needRestart |= key.IsCreated();

    //update config values
    Optional<bool> result = UpdateIpOverUsbConfig(key);
    DVASSERT_MSG_RET(result.IsSet(), "Unable to update IpOverUsb service config", false);
    needRestart |= result.Get();

    //restart service to applying new config
    if (needRestart)
        return RestartIpOverUsb();
    return true;
}