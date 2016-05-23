#include "CommandLine/CommandLineManager.h"
#include "CommandLine/CommandLineTool.h"

#include "CommandLine/ImageSplitter/ImageSplitterTool.h"
#include "CommandLine/SceneSaver/SceneSaverTool.h"
#include "CommandLine/SceneExporter/SceneExporterTool.h"
#include "CommandLine/StaticOcclusion/StaticOcclusionTool.h"
#include "CommandLine/Dump/DumpTool.h"
#include "CommandLine/Beast/BeastCommandLineTool.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorTool.h"
#include "CommandLine/Version/VersionTool.h"

#include "Main/QtUtils.h"

using namespace DAVA;

CommandLineManager::CommandLineManager(int argc, char* argv[])
    : helpOption("help")
{
    CreateTools();
    ParseCommandLine(argc, argv);
}

void CommandLineManager::CreateTools()
{
    commandLineTools.emplace_back(new SceneExporterTool());
    commandLineTools.emplace_back(new SceneSaverTool());
    commandLineTools.emplace_back(new TextureDescriptorTool());
    commandLineTools.emplace_back(new StaticOcclusionTool());
    commandLineTools.emplace_back(new DumpTool());
    commandLineTools.emplace_back(new ImageSplitterTool());
#if defined(__DAVAENGINE_BEAST__)
    commandLineTools.emplace_back(new BeastCommandLineTool());
#endif //#if defined (__DAVAENGINE_BEAST__)
    commandLineTools.emplace_back(new VersionTool());
}

CommandLineManager::~CommandLineManager() = default;

void CommandLineManager::ParseCommandLine(int argc, char* argv[])
{
    isConsoleModeEnabled = Core::Instance()->IsConsoleMode();

    helpRequested = helpOption.Parse(argc, argv);
    if (helpRequested)
    {
        isConsoleModeEnabled = true;
    }
    else
    {
        for (auto& cmdTool : commandLineTools)
        {
            auto parseResult = cmdTool->ParseCommandLine(argc, argv);
            if (parseResult)
            {
                activeTool = cmdTool.get();
                isConsoleModeEnabled = true;
                break;
            }
        }
    }
}

void CommandLineManager::Process()
{
    if (helpRequested)
    {
        PrintUsage();
        return;
    }

    if (activeTool != nullptr)
    {
        activeTool->Process();
    }
}

void CommandLineManager::PrintUsage()
{
    printf("Usage: ResourceEditor <command>\n");
    printf("\n Commands: ");

    for (auto& cmdTool : commandLineTools)
    {
        printf("%s, ", cmdTool->GetToolKey().c_str());
    }
    printf("%s", helpOption.GetCommand().c_str());

    printf("\n\n");
    for (auto& cmdTool : commandLineTools)
    {
        cmdTool->PrintUsage();
        printf("\n");
    }
}
