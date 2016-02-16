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

#include "CommandLine/CommandLineManager.h"
#include "CommandLine/CommandLineTool.h"

#include "CommandLine/ImageSplitter/ImageSplitterTool.h"
#include "CommandLine/SceneSaver/SceneSaverTool.h"
#include "CommandLine/SceneExporter/SceneExporterTool.h"
#include "CommandLine/StaticOcclusion/StaticOcclusionTool.h"
#include "CommandLine/Dump/DumpTool.h"
#include "CommandLine/Beast/BeastCommandLineTool.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorTool.h"

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
}

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
