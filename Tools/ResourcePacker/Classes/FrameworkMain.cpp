#include "DAVAEngine.h"
#include "GameCore.h"

#include "Utils/Utils.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "CommandLine/CommandLineParser.h"

#include "TextureCompression/PVRConverter.h"

#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"
#include "Logger/TeamcityOutput.h"

#include "AssetCache/AssetCacheClient.h"

using namespace DAVA;

void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n");
    printf("\t-v or --verbose - detailed output\n");
    printf("\t-s or --silent - silent mode. Log only warnings and errors.\n");
    printf("\t-teamcity - extra output in teamcity format\n");
    printf("\t-md5mode - only process md5 for output resources\n");
    printf("\t-useCache - use asset cache\n");
    printf("\t-ip - asset cache ip\n");
    printf("\t-p - asset cache port\n");
    printf("\t-t - asset cache timeout\n");
    printf("\t-output - output folder for .../Project/Data/Gfx/\n");

    printf("\n");
    printf("resourcepacker [src_dir] - will pack resources from src_dir\n");
}

void DumpCommandLine()
{
    const Vector<String>& commandLine = Core::Instance()->GetCommandLine();

    Logger::FrameworkDebug("");
    for (auto& param : commandLine)
    {
        Logger::FrameworkDebug("parameter: %s", param.c_str());
    }
    Logger::FrameworkDebug("");
}

void ProcessRecourcePacker()
{
    if (CommandLineParser::Instance()->GetVerbose())
    {
        DumpCommandLine();
    }

    ResourcePacker2D resourcePacker;

    auto& commandLine = Core::Instance()->GetCommandLine();
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

    if (resourcePacker.rootDirectory.GetLastDirectoryName() != "DataSource")
    {
        Logger::Error("[FATAL ERROR: Packer working only inside DataSource directory]");
        return;
    }

    if (commandLine.size() < 3)
    {
        Logger::Error("[FATAL ERROR: PVRTexTool path need to be second parameter]");
        return;
    }

    auto toolFolderPath = resourcePacker.rootDirectory + (commandLine[2] + "/");
    String pvrTexToolName = "PVRTexToolCLI";
    String cacheToolName = "AssetCacheClient";

    PVRConverter::Instance()->SetPVRTexTool(toolFolderPath + pvrTexToolName);

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

    AssetCacheClient cacheClient(true);
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

void FrameworkDidLaunched()
{
    Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);

    if (Core::Instance()->IsConsoleMode())
    {
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

            Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);
        }

        if (CommandLineParser::CommandIsFound(String("-v")) || CommandLineParser::CommandIsFound(String("--verbose")))
        {
            CommandLineParser::Instance()->SetVerbose(true);

            Logger::Instance()->SetLogLevel(Logger::LEVEL_FRAMEWORK);
        }

        if (CommandLineParser::CommandIsFound(String("-s")) || CommandLineParser::CommandIsFound(String("--silent")))
        {
            Logger::Instance()->SetLogLevel(Logger::LEVEL_WARNING);
        }

        if (CommandLineParser::CommandIsFound(String("-teamcity")))
        {
            CommandLineParser::Instance()->SetUseTeamcityOutput(true);

            DAVA::TeamcityOutput* out = new DAVA::TeamcityOutput();
            DAVA::Logger::AddCustomOutput(out);
        }
    }

    ProcessRecourcePacker();
}

void FrameworkWillTerminate()
{
}
