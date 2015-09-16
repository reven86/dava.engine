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


#include <QFile>
#include <QXmlStreamReader>

#include "Concurrency/LockGuard.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/Thread.h"
#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Network/Services/LogConsumer.h"
#include "Network/SimpleNetworking/SimpleNetCore.h"
#include "Network/SimpleNetworking/SimpleNetService.h"

#include <Objbase.h>

#include "AppxBundleHelper.h"
#include "ArchiveExtraction.h"
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

void LaunchPackage(const PackageOptions& opt);
void LaunchPackage(const FilePath& package, const PackageOptions& opt);

String GetCurrentArchitecture();
QString GetQtWinRTRunnerProfile(const Optional<String>& profile, const FilePath& manifest);
FilePath ExtractManifest(const FilePath& package);

void FrameworkDidLaunched()
{
    //Parse arguments
    PackageOptions commandLineOptions = ParseCommandLine();
    if (!CheckOptions(commandLineOptions))
    {
        return;
    }

    LaunchPackage(commandLineOptions);
}

void LaunchPackage(const PackageOptions& opt)
{
    FilePath package = opt.package.Get();

    //if package is bundle, extract concrete package from it
    if (AppxBundleHelper::IsBundle(package))
    {
        Logger::Instance()->Info("Extracting package from bundle...");
        AppxBundleHelper bundle(package);

        //try to extract package for specified architecture
        if (opt.architecture.IsSet())
        {
            package = bundle.ExtractApplicationForArchitecture(opt.architecture.Get());
            if (package.IsEmpty())
            {
                DVASSERT_MSG(false, "Can't extract package for specified architecture from bundle");
            }
        }
        //try to extract package for current architecture
        else
        {
            package = bundle.ExtractApplicationForArchitecture(GetCurrentArchitecture());

            //try to extract package for any architecture
            if (package.IsEmpty())
            {
                Vector<AppxBundleHelper::PackageInfo> applications = bundle.GetApplications();
                package = bundle.ExtractApplication(applications.at(0).name);
            }
        }

        if (!package.IsEmpty())
        {
            LaunchPackage(package, opt);
        }
    }
    else
    {
        LaunchPackage(package, opt);
    }
}

void LaunchPackage(const FilePath& package, const PackageOptions& opt)
{
    //Extract manifest from package
    Logger::Instance()->Info("Extracting manifest...");
    FilePath manifest = ExtractManifest(package);
    if (manifest.IsEmpty())
    {
        DVASSERT_MSG(false, "Can't extract manifest file from package");
        return;
    }

    SCOPE_EXIT
    {
        FileSystem::Instance()->DeleteFile(manifest);
    };

    //figure out if app should be started on mobile device
    QString profile = GetQtWinRTRunnerProfile(opt.profile, manifest);
    bool isMobileDevice = profile == QStringLiteral("appxphone");

    //Init network
    Logger::Instance()->Info("Initializing network...");
    if (!InitializeNetwork(isMobileDevice))
    {
        DVASSERT_MSG(false, "Unable to initialize network");
        return;
    }

    //Create Qt runner
    Logger::Instance()->Info("Preparing to launch...");
    Runner runner(QString::fromStdString(package.GetAbsolutePathname()),
                  QString::fromStdString(manifest.GetAbsolutePathname()),
                  QString::fromStdString(opt.dependencies.Get()),
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
    //Start app
    Start(runner);

    //Wait app exit
    WaitApp();

    //remove app package after working
    DVASSERT_MSG_RET(runner.remove(), "Unable to remove package");
}

void Start(Runner& runner)
{
    Logger::Instance()->Info("Installing package...");
    DVASSERT_MSG_RET(runner.install(true), "Can't install application package");

    Logger::Instance()->Info("Starting application...");
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
    Logger::Instance()->Info("Waiting app exit...");

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

String GetCurrentArchitecture()
{
    return sizeof(void*) == 4 ? "x86" : "x64";
}

QString GetQtWinRTRunnerProfile(const Optional<String>& profile, const FilePath& manifest)
{
    //if profile is set, just convert it
    if (profile.IsSet())
    {
        return profile == "local" ? QStringLiteral("appx") : QStringLiteral("appxphone");
    }

    //else try to find out profile from manifest
    QFile file(QString::fromStdString(manifest.GetAbsolutePathname()));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token != QXmlStreamReader::StartElement ||
            xml.name() != QStringLiteral("Identity"))
        {
            continue;
        }

        QXmlStreamAttributes attributes = xml.attributes();
        for (const auto& attribute : attributes)
        {
            if (attribute.name() == QStringLiteral("ProcessorArchitecture"))
            {
                QString arch = attribute.value().toString().toLower();
                if (arch == QStringLiteral("arm"))
                {
                    return QStringLiteral("appxphone");
                }
                else
                {
                    return QStringLiteral("appx");
                }
            }
        }
    }

    return "";
}

FilePath ExtractManifest(const FilePath& package)
{
    FilePath manifestFilePath = GetTempFileName();

    //extract manifest from appx
    if (ExtractFileFromArchive(package.GetAbsolutePathname(),
                               "AppxManifest.xml",
                               manifestFilePath.GetAbsolutePathname()))
    {
        return manifestFilePath;
    }
    return FilePath();
}