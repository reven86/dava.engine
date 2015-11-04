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

#include "Concurrency/Atomic.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/Thread.h"
#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Network/Services/LogConsumer.h"
#include "Network/SimpleNetworking/SimpleNetCore.h"
#include "Network/SimpleNetworking/SimpleNetService.h"
#include "TeamcityOutput/TeamCityTestsOutput.h"

#include <Objbase.h>

#include "AppxBundleHelper.h"
#include "ArchiveExtraction.h"
#include "SvcHelper.h"
#include "runner.h"
#include "RegKey.h"
#include "UWPRunner.h"

using namespace DAVA;

const Net::SimpleNetService* gNetLogger = nullptr;
Atomic<bool> interrupt = false;
using StringRecv = Function<void(const String&)>;

void Run(Runner& runner);
void Start(Runner& runner);

bool InitializeNetwork(bool isMobileDevice, const StringRecv& logReceiver);
void LogConsumingFunction(bool useTeamCityTestOutput, const String& logString);
bool ConfigureIpOverUsb();
void WaitApp();

void LaunchPackage(PackageOptions opt);
void LaunchAppPackage(const PackageOptions& opt);

String GetCurrentArchitecture();
QString GetQtWinRTRunnerProfile(const String& profile, const FilePath& manifest);
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

void LaunchPackage(PackageOptions opt)
{
    //if package is bundle, extract concrete package from it
    if (!AppxBundleHelper::IsBundle(opt.mainPackage))
    {
        opt.packageToInstall = opt.mainPackage;
        LaunchAppPackage(opt);
        return;
    }

    FilePath package = opt.mainPackage;
    AppxBundleHelper bundle(package);

    //try to extract package for specified architecture
    if (!opt.architecture.empty())
    {
        package = bundle.GetApplicationForArchitecture(opt.architecture);
    }
    //try to extract package for current architecture
    else
    {
        package = bundle.GetApplicationForArchitecture(GetCurrentArchitecture());

        //try to extract package for any architecture
        if (package.IsEmpty())
        {
            Vector<AppxBundleHelper::PackageInfo> applications = bundle.GetApplications();
            package = bundle.GetApplication(applications.at(0).name);
        }
    }

    DVASSERT_MSG(!package.IsEmpty(), "Can't extract app package from bundle");
    if (!package.IsEmpty())
    {
        Vector<AppxBundleHelper::PackageInfo> resources = bundle.GetResources();
        for (const auto& x : resources)
        {
            opt.resources.push_back(x.path.GetAbsolutePathname());
        }

        opt.packageToInstall = package.GetAbsolutePathname();
        LaunchAppPackage(opt);
    }
}

void LaunchAppPackage(const PackageOptions& opt)
{
    //Extract manifest from package
    Logger::Instance()->Info("Extracting manifest...");
    FilePath manifest = ExtractManifest(opt.packageToInstall);
    if (manifest.IsEmpty())
    {
        DVASSERT_MSG(false, "Can't extract manifest file from package");
        return;
    }

    //figure out if app should be started on mobile device
    QString profile = GetQtWinRTRunnerProfile(opt.profile, manifest);
    FileSystem::Instance()->DeleteFile(manifest);
    bool isMobileDevice = profile == QStringLiteral("appxphone");

    //Init network
    Logger::Instance()->Info("Initializing network...");

    auto logConsumer = [=](const String& logString) 
    { 
        LogConsumingFunction(opt.useTeamCityTestOutput, logString); 
    };

    if (!InitializeNetwork(isMobileDevice, logConsumer))
    {
        DVASSERT_MSG(false, "Unable to initialize network");
        return;
    }

    //Create Qt runner
    Logger::Instance()->Info("Preparing to launch...");

    QStringList resources;
    for (const auto& x : opt.resources)
    {
        resources.push_back(QString::fromStdString(x));
    }

    Runner runner(QString::fromStdString(opt.mainPackage),
                  QString::fromStdString(opt.packageToInstall),
                  resources,
                  QString::fromStdString(opt.dependencies),
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
    if (!runner.remove())
    {
        DVASSERT_MSG(false, "Unable to remove package");
    }
}

void Start(Runner& runner)
{
    Logger::Instance()->Info("Installing package...");
    if (!runner.install(true))
    {
        DVASSERT_MSG(false, "Can't install application package");
    }

    Logger::Instance()->Info("Starting application...");
    if (!runner.start())
    {
        DVASSERT_MSG(false, "Can't install application package");
    }
}

bool InitializeNetwork(bool isMobileDevice, const StringRecv& logReceiver)
{
    if (isMobileDevice)
    {
        if (!ConfigureIpOverUsb())
        {
            DVASSERT_MSG(false, "Can't configure IpOverUsb service");
            return false;
        }
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

    auto logConsumer = std::make_unique<Net::LogConsumer>();
    logConsumer->newDataNotifier.Connect(logReceiver);

    Net::SimpleNetCore* netcore = new Net::SimpleNetCore;
    gNetLogger = netcore->RegisterService(
        std::move(logConsumer), role, endPoint, "RawLogConsumer", false);

    return gNetLogger != nullptr;
}

void WaitApp()
{
    Logger::Instance()->Info("Waiting app exit...");

    /*while (true)
    {
        
        Thread::Sleep(1000);
        if (!gNetLogger->IsActive())
        {
            break;
        }
    }*/

    while (!interrupt) { Thread::Sleep(1000); }
}

void FrameworkWillTerminate()
{
    //cleanup network
    Net::SimpleNetCore::Instance()->Release();
}

bool UpdateIpOverUsbConfig(RegKey& key)
{
    const String desiredDestAddr = "127.0.0.1";
    const DWORD  desiredDestPort = Net::SimpleNetCore::UWPRemotePort;
    const String desiredLocalAddr = desiredDestAddr;
    const DWORD  desiredLocalPort = desiredDestPort;
    bool changed = false;

    String address = key.QueryString("DestinationAddress");
    if (address != desiredDestAddr)
    {
        if (!key.SetValue("DestinationAddress", desiredDestAddr))
        {
            DVASSERT_MSG(false, "Unable to set DestinationAddress");
            return false;
        }
        changed |= true;
    }

    DWORD port = key.QueryDWORD("DestinationPort");
    if (port != desiredDestPort)
    {
        if (!key.SetValue("DestinationPort", desiredDestPort))
        {
            DVASSERT_MSG(false, "Unable to set DestinationPort");
            return false;
        }
        changed |= true;
    }

    address = key.QueryString("LocalAddress");
    if (address != desiredLocalAddr)
    {
        if (!key.SetValue("LocalAddress", desiredLocalAddr))
        {
            DVASSERT_MSG(false, "Unable to set LocalAddress");
            return false;
        }
        changed |= true;
    }

    port = key.QueryDWORD("LocalPort");
    if (port != desiredLocalPort)
    {
        if (!key.SetValue("LocalPort", desiredLocalPort))
        {
            DVASSERT_MSG(false, "Unable to set LocalPort");
            return false;
        }
        changed |= true;
    }

    return changed;
}

bool RestartIpOverUsb()
{
    //open service
    SvcHelper service("IpOverUsbSvc");
    if (!service.IsInstalled())
    {
        DVASSERT_MSG(false, "Can't open IpOverUsb service");
        return false;
    }

    //stop it
    if (!service.Stop())
    {
        DVASSERT_MSG(false, "Can't stop IpOverUsb service");
        return false;
    }
    
    //start it
    if (!service.Start())
    {
        DVASSERT_MSG(false, "Can't start IpOverUsb service");
        return false;
    }

    //waiting for service starting
    Thread::Sleep(1000);

    return true;
}

bool ConfigureIpOverUsb()
{
    bool needRestart = false;

    //open or create key
    RegKey key(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\IpOverUsbSdk\\DavaDebugging", true);
    if (!key.IsExist())
    {
        DVASSERT_MSG(false, "Can't open or create key");
        return false;
    }
    needRestart |= key.IsCreated();

    //update config values
    bool result = UpdateIpOverUsbConfig(key);
    /*if (!result.IsSet())
    {
        DVASSERT_MSG(false, "Unable to update IpOverUsb service config");
        return false;
    }*/
    needRestart |= result;

    //restart service to applying new config
    if (needRestart)
        return RestartIpOverUsb();
    return true;
}

String GetCurrentArchitecture()
{
    return sizeof(void*) == 4 ? "x86" : "x64";
}

QString GetQtWinRTRunnerProfile(const String& profile, const FilePath& manifest)
{
    //if profile is set, just convert it
    if (!profile.empty())
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

void LogConsumingFunction(bool useTeamCityTestOutput, const String& logString)
{
    //incoming string is formatted in style "[ip:port] date time message"
    //extract only message text
    String logLevel;
    String message;

    size_t spaces = 0;
    for (auto i : logString)
    {
        if (::isspace(i))
        {
            spaces++;
        }

        if (spaces == 3)
        {
            logLevel += i;
        }
        else if (spaces >= 4)
        {
            message += i;
        }
    }

    //remove first space
    logLevel = logLevel.substr(1);
    message = message.substr(1);

    if (useTeamCityTestOutput)
    {
        Logger* logger = Logger::Instance();
        Logger::eLogLevel ll = logger->GetLogLevelFromString(logLevel.c_str());

        if (ll != Logger::LEVEL__DISABLE)
        {
            TeamcityTestsOutput testOutput;
            testOutput.Output(ll, message.c_str());

            if (message.find("Finish all tests") != String::npos)
            {
                interrupt = true;
            }
        }

    }
    else
    {
        printf("[%s] %s", logLevel.c_str(), message.c_str());
    }
}