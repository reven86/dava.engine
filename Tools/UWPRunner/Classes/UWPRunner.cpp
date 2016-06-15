#include <iostream>
#include <QFile>
#include <QXmlStreamReader>

#include "Concurrency/Thread.h"
#include "FileSystem/FileSystem.h"
#include "Functional/Function.h"
#include "Network/NetConfig.h"
#include "Network/Services/LogConsumer.h"
#include "Platform/TemplateWin32/UAPNetworkHelper.h"
#include "Logger/TeamCityTestsOutput.h"
#include "Utils/Utils.h"

#include "AppxBundleHelper.h"
#include "ArchiveExtraction.h"
#include "SvcHelper.h"
#include "runner.h"
#include "RegKey.h"
#include "UWPLogConsumer.h"
#include "UWPRunner.h"

using namespace DAVA;

String GetCurrentArchitecture();
QString GetQtWinRTRunnerProfile(const String& profile, const FilePath& manifest);
FilePath ExtractManifest(const FilePath& package);

void ThrowException(int line, const char* func, const char* file, const char* msg)
{
    StringStream ss;
    ss << "Exception in " << func << " in " << file << "::" << line << ": " << msg;
    throw std::runtime_error(ss.str());
}

#define RUNNER_EXCEPTION(msg) ThrowException(__LINE__, __FUNCTION__, __FILE__, msg);

UWPRunner::UWPRunner(const PackageOptions& opt)
    : options(opt)
{
    //install slot to log consumer
    logConsumerConnectionID = logConsumer.newMessageNotifier.Connect([this](const String& logStr) {
        NetLogOutput(logStr);
    });
}

UWPRunner::~UWPRunner()
{
    cleanNeeded.Emit();
    logConsumer.newMessageNotifier.Disconnect(logConsumerConnectionID);

    if (!options.installOnly)
    {
        UnInitializeNetwork();
    }
}

void UWPRunner::Run()
{
    //Create Qt runner
    Logger::Info("Preparing to launch...");
    ProcessPackageOptions();
    ProcessProfileInfo();

    //Init network
    if (!options.installOnly)
    {
        Logger::Info("Initializing network...");
        InitializeNetwork(qtProfile == "appxphone");
    }

    QStringList resources;
    for (const auto& x : options.resources)
    {
        resources.push_back(QString::fromStdString(x));
    }

    Runner runner(QString::fromStdString(options.mainPackage),
                  QString::fromStdString(options.packageToInstall),
                  resources,
                  QString::fromStdString(options.dependencies),
                  QStringList(),
                  QString::fromStdString(qtProfile));

    //Check runner state
    if (!runner.isValid())
    {
        RUNNER_EXCEPTION("Runner core is not valid");
    }

    Run(runner);
}

bool UWPRunner::IsSucceed()
{
    return succeed;
}

void UWPRunner::Run(Runner& runner)
{
    //installing and starting application
    if (!options.runOnly)
    {
        Logger::Info("Installing package...");
        if (!runner.install(true))
        {
            RUNNER_EXCEPTION("Can't install application package");
            return;
        }
    }

    if (options.installOnly)
    {
        succeed = true;
        return;
    }

    Logger::Info("Starting application...");
    if (!runner.start())
    {
        RUNNER_EXCEPTION("Can't install application package");
        return;
    }

    //post-start cleaning
    cleanNeeded.Emit();

    //wait application exit
    WaitApp();
}

void UWPRunner::WaitApp()
{
    const size_t connectionTimeout = 5 * 60 * 1000;
    const uint32 sleepTimeMS = 10;
    size_t watchDogTimer = 0;

    Logger::Info("Waiting application exit...");

    do
    {
        Net::NetCore::Instance()->Poll();

        if (logConsumer.IsChannelOpen())
        {
            succeed = true;
            watchDogTimer = 0;
        }
        else
        {
            watchDogTimer += sleepTimeMS;
            if (watchDogTimer >= connectionTimeout)
            {
                Logger::Error("Cannot connect to application");
                break;
            }
        }

        Thread::Sleep(sleepTimeMS);
    } while (!logConsumer.IsSessionEnded());

    if (succeed && options.isDavaApplication)
    {
        succeed = davaApplicationTerminated;
    }
}

void UWPRunner::ProcessPackageOptions()
{
    //if package is bundle, extract concrete package from it
    if (AppxBundleHelper::IsBundle(options.mainPackage))
    {
        ProcessBundlePackage();
    }
    else
    {
        options.packageToInstall = options.mainPackage;
    }
}

void UWPRunner::ProcessBundlePackage()
{
    FilePath package = options.mainPackage;
    bundleHelper.reset(new AppxBundleHelper(package));
    cleanNeeded.Connect([this] { bundleHelper.reset(); });

    //try to extract package for specified architecture
    if (!options.architecture.empty())
    {
        package = bundleHelper->GetApplicationForArchitecture(options.architecture);
    }
    //try to extract package for current architecture
    else
    {
        package = bundleHelper->GetApplicationForArchitecture(GetCurrentArchitecture());

        //try to extract package for any architecture
        if (package.IsEmpty())
        {
            Vector<AppxBundleHelper::PackageInfo> applications = bundleHelper->GetApplications();
            package = bundleHelper->GetApplication(applications.at(0).name);
        }
    }

    if (!package.IsEmpty())
    {
        Vector<AppxBundleHelper::PackageInfo> resources = bundleHelper->GetResources();
        for (const auto& x : resources)
        {
            options.resources.push_back(x.path.GetAbsolutePathname());
        }

        options.packageToInstall = package.GetAbsolutePathname();
    }
    else
    {
        RUNNER_EXCEPTION("Can't extract app package from bundle");
    }
}

void UWPRunner::ProcessProfileInfo()
{
    //Extract manifest from package
    Logger::Info("Extracting manifest...");
    FilePath manifest = ExtractManifest(options.packageToInstall);
    if (manifest.IsEmpty())
    {
        RUNNER_EXCEPTION("Can't extract manifest file from package");
    }

    //figure out if app should be started on mobile device
    qtProfile = GetQtWinRTRunnerProfile(options.profile, manifest).toStdString();
    FileSystem::Instance()->DeleteFile(manifest);
}

void UWPRunner::InitializeNetwork(bool isMobileDevice)
{
    using namespace Net;

    if (isMobileDevice)
    {
        bool ipOverUsbConfigured = ConfigureIpOverUsb();
        if (!ipOverUsbConfigured)
        {
            RUNNER_EXCEPTION("Cannot configure IpOverUSB service");
        }
    }

    NetCore::Instance()->RegisterService(
    NetCore::SERVICE_LOG,
    [this](uint32 serviceId, void*) -> IChannelListener* { return &logConsumer; },
    [](IChannelListener* obj, void*) -> void {});

    eNetworkRole role;
    Endpoint endPoint;
    if (isMobileDevice)
    {
        role = CLIENT_ROLE;
        endPoint = Endpoint(UAPNetworkHelper::UAP_IP_ADDRESS, UAPNetworkHelper::UAP_MOBILE_TCP_PORT);
    }
    else
    {
        role = SERVER_ROLE;
        endPoint = Endpoint(UAPNetworkHelper::UAP_DESKTOP_TCP_PORT);
    }

    NetConfig config(role);
    config.AddTransport(TRANSPORT_TCP, endPoint);
    config.AddService(NetCore::SERVICE_LOG);

    const uint32 timeout = 5 * 60 * 1000; //5 min
    controllerId = NetCore::Instance()->CreateController(config, nullptr, timeout);
}

void UWPRunner::UnInitializeNetwork()
{
    if (controllerId != Net::NetCore::INVALID_TRACK_ID)
    {
        Net::NetCore* netCore = Net::NetCore::Instance();
        netCore->DestroyControllerBlocked(controllerId);
        netCore->UnregisterService(Net::NetCore::SERVICE_LOG);
        controllerId = Net::NetCore::INVALID_TRACK_ID;
    }
}

bool UWPRunner::UpdateIpOverUsbConfig(RegKey& key)
{
    const WideString desiredAddr = StringToWString(UAPNetworkHelper::UAP_IP_ADDRESS);
    const DWORD desiredPort = UAPNetworkHelper::UAP_MOBILE_TCP_PORT;
    bool changed = false;

    WideString address = key.QueryString(L"DestinationAddress");
    if (address != desiredAddr)
    {
        if (!key.SetValue(L"DestinationAddress", desiredAddr))
        {
            RUNNER_EXCEPTION("Unable to set DestinationAddress");
        }
        changed = true;
    }

    DWORD port = key.QueryDWORD(L"DestinationPort");
    if (port != desiredPort)
    {
        if (!key.SetValue(L"DestinationPort", desiredPort))
        {
            RUNNER_EXCEPTION("Unable to set DestinationPort");
        }
        changed = true;
    }

    address = key.QueryString(L"LocalAddress");
    if (address != desiredAddr)
    {
        if (!key.SetValue(L"LocalAddress", desiredAddr))
        {
            RUNNER_EXCEPTION("Unable to set LocalAddress");
        }
        changed = true;
    }

    port = key.QueryDWORD(L"LocalPort");
    if (port != desiredPort)
    {
        if (!key.SetValue(L"LocalPort", desiredPort))
        {
            RUNNER_EXCEPTION("Unable to set LocalPort");
        }
        changed = true;
    }

    return changed;
}

bool UWPRunner::RestartIpOverUsb()
{
    //open service
    SvcHelper service(L"IpOverUsbSvc");
    if (!service.IsInstalled())
    {
        RUNNER_EXCEPTION("Can't open IpOverUsb service");
    }

    //stop it
    if (!service.Stop())
    {
        RUNNER_EXCEPTION("Can't stop IpOverUsb service");
    }

    //start it
    if (!service.Start())
    {
        RUNNER_EXCEPTION("Can't start IpOverUsb service");
    }

    return true;
}

bool UWPRunner::ConfigureIpOverUsb()
{
    bool needRestart = false;

    //open or create key
    RegKey key(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\IpOverUsbSdk\\DavaDebugging", true);
    if (!key.IsExist())
    {
        RUNNER_EXCEPTION("Can't open or create key");
    }
    needRestart |= key.IsCreated();

    //update config values
    bool result = UpdateIpOverUsbConfig(key);
    needRestart |= result;

    //restart service to applying new config
    if (needRestart)
        return RestartIpOverUsb();
    return true;
}

void SplitLoggerMessage(const String& logString, String& logLevel, String& message)
{
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
}

void TeamcityTestOutputFunc(const char* logLevelStr, const char* messageStr)
{
    Logger* logger = Logger::Instance();
    Logger::eLogLevel ll = logger->GetLogLevelFromString(logLevelStr);

    if (ll != Logger::LEVEL__DISABLE)
    {
        TeamcityTestsOutput testOutput;
        testOutput.Output(ll, messageStr);
    }
}

void UWPRunner::NetLogOutput(const String& logString)
{
    const char* davaAppTermString = "Core::SystemAppFinished";

    //incoming string is formatted in style "[ip:port] date time message"
    //extract only message text
    String logLevel;
    String message;
    SplitLoggerMessage(logString, logLevel, message);

    if (logLevel.empty())
    {
        return;
    }

    //remove first space
    const char* logLevelStr = logLevel.c_str() + 1;
    const char* messageStr = message.c_str() + 1;

    if (options.useTeamCityTestOutput)
    {
        TeamcityTestOutputFunc(logLevelStr, messageStr);
    }
    else if (!options.useTeamCityTestOutput || !options.outputFile.empty())
    {
        StringStream ss;
        ss << "[" << logLevelStr << "] " << messageStr;
        if (message.back() != '\n' || message.back() != '\r')
        {
            ss << "\n";
        }

        std::cout << ss.str();

        if (!options.outputFile.empty())
        {
            if (!outputFile)
            {
                FileSystem::Instance()->DeleteFile(options.outputFile);
                uint32 attributes = File::WRITE;
                outputFile.Set(File::Create(options.outputFile, attributes));
            }

            if (outputFile)
            {
                outputFile->WriteString(ss.str(), false);
                outputFile->Flush();
            }
        }
    }

    if (message.find(davaAppTermString) != String::npos)
    {
        davaApplicationTerminated = true;
    }
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
    String manifestFilePath = package.GetAbsolutePathname() + "_manifest.xml";

    //extract manifest from appx
    if (ExtractFileFromArchive(package.GetAbsolutePathname(),
                               "AppxManifest.xml",
                               manifestFilePath))
    {
        return manifestFilePath;
    }
    return FilePath();
}