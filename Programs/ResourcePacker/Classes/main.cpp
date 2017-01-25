#include "TexturePacker/ResourcePacker2D.h"
#include "TextureCompression/PVRConverter.h"

#include "Engine/Engine.h"
#include "CommandLine/CommandLineParser.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Logger/Logger.h"
#include "Logger/TeamcityOutput.h"
#include "Debug/DVAssertDefaultHandlers.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

using namespace DAVA;

void PrintUsage()
{
    printf("Usage:\n");

    printf("ResourcePacker {src_dir} -pvrTexToolFolder {path_to_PVRTexToolCLI} [options]\n\n");

    printf("Options:\n");
    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n");
    printf("\t-v or --verbose - detailed output\n");
    printf("\t-s or --silent - silent mode. Log only warnings and errors.\n");
    printf("\t-teamcity - extra output in teamcity format\n");
    printf("\t-md5mode - only process md5 for output resources\n");
    printf("\t-useCache - use asset cache\n");
    printf("\t-ip - asset cache ip\n");
    printf("\t-t - asset cache timeout\n");
    printf("\t-postifx - trailing part of texture name\n");
    printf("\t-output - output folder for .../Project/Data/Gfx/\n");

    printf("\n");
    printf("ResourcePacker [src_dir] - will pack resources from src_dir\n");
}

void DumpCommandLine(Engine& e)
{
    const Vector<String>& commandLine = e.GetCommandLine();

    Logger::FrameworkDebug("");
    for (auto& param : commandLine)
    {
        Logger::FrameworkDebug("parameter: %s", param.c_str());
    }
    Logger::FrameworkDebug("");
}

void ProcessRecourcePacker(Engine& e)
{
    if (CommandLineParser::Instance()->GetVerbose())
    {
        DumpCommandLine(e);
    }

    ResourcePacker2D resourcePacker;

    const Vector<String>& commandLine = e.GetCommandLine();
    FilePath inputDir(commandLine[1]);
    inputDir.MakeDirectoryPathname();

    String lastDir = inputDir.GetDirectory().GetLastDirectoryName();

    FilePath outputDir = CommandLineParser::GetCommandParam("-output");
    if (outputDir.IsEmpty())
    {
        outputDir = inputDir + ("../../Data/" + lastDir + "/");
    }
    outputDir.MakeDirectoryPathname();
    resourcePacker.InitFolders(inputDir, outputDir);

    if (resourcePacker.rootDirectory.IsEmpty())
    {
        Logger::Error("[FATAL ERROR: Packer has wrong input pathname]");
        return;
    }

    FilePath pvrTexToolFolder = CommandLineParser::GetCommandParam("-pvrTexToolPath");
    if (pvrTexToolFolder.IsEmpty())
    {
        Logger::Error("[FATAL ERROR: '-pvrTexToolPath' param should be specified]");
        return;
    }

    pvrTexToolFolder.MakeDirectoryPathname();
    const String pvrTexToolName = "PVRTexToolCLI";

    PVRConverter::Instance()->SetPVRTexTool(pvrTexToolFolder + pvrTexToolName);

    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS();
    Logger::FrameworkDebug("[Resource Packer Started]");
    Logger::FrameworkDebug("[INPUT DIR] - [%s]", resourcePacker.inputGfxDirectory.GetAbsolutePathname().c_str());
    Logger::FrameworkDebug("[OUTPUT DIR] - [%s]", resourcePacker.outputGfxDirectory.GetAbsolutePathname().c_str());
    Logger::FrameworkDebug("[EXCLUDE DIR] - [%s]", resourcePacker.rootDirectory.GetAbsolutePathname().c_str());

    Vector<eGPUFamily> exportForGPUs;
    if (CommandLineParser::CommandIsFound(String("-gpu")))
    {
        String gpuNamesString = CommandLineParser::GetCommandParam("-gpu");
        Vector<String> gpuNames;
        Split(gpuNamesString, ",", gpuNames);

        for (String& name : gpuNames)
        {
            exportForGPUs.push_back(GPUFamilyDescriptor::GetGPUByName(name));
        }
    }

    if (exportForGPUs.empty())
    {
        exportForGPUs.push_back(GPU_ORIGIN);
    }

    AssetCacheClient cacheClient;
    bool shouldDisconnect = false;
    if (CommandLineParser::CommandIsFound(String("-useCache")))
    {
        Logger::FrameworkDebug("Using asset cache");

        String ipStr = CommandLineParser::GetCommandParam("-ip");
        String portStr = CommandLineParser::GetCommandParam("-p");
        String timeoutStr = CommandLineParser::GetCommandParam("-t");

        AssetCacheClient::ConnectionParams params;
        params.ip = (ipStr.empty() ? AssetCache::GetLocalHost() : ipStr);
        params.port = (portStr.empty()) ? AssetCache::ASSET_SERVER_PORT : atoi(portStr.c_str());
        params.timeoutms = (timeoutStr.empty() ? 1000 : atoi(timeoutStr.c_str()) * 1000); //in ms

        AssetCache::Error connected = cacheClient.ConnectSynchronously(params);
        if (connected == AssetCache::Error::NO_ERRORS)
        {
            shouldDisconnect = true;
            resourcePacker.SetCacheClient(&cacheClient, "Resource Packer. Repack Sprites");
        }
    }
    else
    {
        Logger::FrameworkDebug("Asset cache will not be used");
    }

    resourcePacker.SetTexturePostfix(CommandLineParser::GetCommandParam("-postfix"));

    if (CommandLineParser::CommandIsFound(String("-md5mode")))
    {
        resourcePacker.RecalculateMD5ForOutputDir();
    }
    else
    {
        resourcePacker.PackResources(exportForGPUs);
    }

    if (shouldDisconnect)
    {
        cacheClient.Disconnect();
    }

    elapsedTime = SystemTimer::Instance()->AbsoluteMS() - elapsedTime;
    Logger::FrameworkDebug("[Resource Packer Compile Time: %0.3lf seconds]", static_cast<float64>(elapsedTime) / 1000.0);
}

void Process(Engine& e)
{
    DVASSERT(e.IsConsoleMode() == true);
    Logger* logger = e.GetContext()->logger;
    logger->SetLogLevel(Logger::LEVEL_INFO);

    if (CommandLineParser::GetCommandsCount() < 2
        || (CommandLineParser::CommandIsFound(String("-usage")))
        || (CommandLineParser::CommandIsFound(String("-help")))
        )
    {
        PrintUsage();
        return;
    }

    if (CommandLineParser::CommandIsFound(String("-exo")))
    {
        CommandLineParser::Instance()->SetExtendedOutput(true);

        logger->SetLogLevel(Logger::LEVEL_INFO);
    }

    if (CommandLineParser::CommandIsFound(String("-v")) || CommandLineParser::CommandIsFound(String("--verbose")))
    {
        CommandLineParser::Instance()->SetVerbose(true);

        logger->SetLogLevel(Logger::LEVEL_FRAMEWORK);
    }

    if (CommandLineParser::CommandIsFound(String("-s")) || CommandLineParser::CommandIsFound(String("--silent")))
    {
        logger->SetLogLevel(Logger::LEVEL_WARNING);
    }

    if (CommandLineParser::CommandIsFound(String("-teamcity")))
    {
        CommandLineParser::Instance()->SetUseTeamcityOutput(true);

        DAVA::TeamcityOutput* out = new DAVA::TeamcityOutput();
        DAVA::Logger::AddCustomOutput(out);
    }

    ProcessRecourcePacker(e);
}

int DAVAMain(Vector<String> cmdLine)
{
    Assert::SetupDefaultHandlers();

    Engine e;
    DAVA::Vector<DAVA::String> modules =
    {
      "NetCore", // AssetCacheClient
      "LocalizationSystem" // ResourcePacker2D::SetCacheClient is using DateTime::GetLocalizedTime() to create cache item
    };
    e.Init(eEngineRunMode::CONSOLE_MODE, modules, nullptr);

    e.update.Connect([&e](float32)
                     {
                         Process(e);
                         e.QuitAsync(0);
                     });

    return e.Run();
}
