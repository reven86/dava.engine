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


#include "CommandLineManager.h"
#include "CommandLineTool.h"

#include "ImageSplitter/ImageSplitterTool.h"
#include "SceneUtils/CleanFolderTool.h"
#include "SceneSaver/SceneSaverTool.h"
#include "SceneExporter/SceneExporterTool.h"
#include "DDSExtractor/DDSExtractorTool.h"
#include "StaticOcclusion/StaticOcclusionTool.h"
#include "Dump/DumpTool.h"

#include "Beast/BeastCommandLineTool.h"
#include "TextureDescriptor/TextureDescriptorTool.h"

#include "TexturePacker/CommandLineParser.h"

#include "Main/QtUtils.h"
#include "TeamcityOutput/TeamcityOutput.h"


using namespace DAVA;

void CommandLineManager::PrintUsage()
{
    printf("\nUsage:\n");
    
    printf("\t-h or --help to display this help\n");
    printf("\t-exo - extended output\n");
    printf("\t-v or --verbose - detailed output\n");
    printf("\t-teamcity - extra output in teamcity format\n");
    printf("\t-forceclose - close editor after job would be finished\n");
    
    Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
    for(auto it = commandLineTools.begin(); it != endIT; ++it)
    {
        it->second->PrintUsage();
    }
    
    printf("\n");
}

void CommandLineManager::PrintUsageForActiveTool()
{
    printf("\nUsage:\n");
    
    printf("\t-h or --help to display this help\n");
    printf("\t-exo - extended output\n");
    printf("\t-v or --verbose - detailed output\n");
    printf("\t-teamcity - extra output in teamcity format\n");
    printf("\t-forceclose - close editor after job would be finished\n");
    
    if(activeTool)
        activeTool->PrintUsage();

    printf("\n");
}

CommandLineManager::CommandLineManager()
	: isEnabled(false)
	, activeTool(NULL)
	, isToolInitialized(false)
{
    AddCommandLineTool(new CleanFolderTool());
    AddCommandLineTool(new ImageSplitterTool());
    AddCommandLineTool(new SceneExporterTool());
    AddCommandLineTool(new SceneSaverTool());
	AddCommandLineTool(new DDSExtractorTool());
    AddCommandLineTool(new StaticOcclusionTool());

#if defined (__DAVAENGINE_BEAST__)
	AddCommandLineTool(new BeastCommandLineTool());
#endif //#if defined (__DAVAENGINE_BEAST__)
    
    AddCommandLineTool(new TextureDescriptorTool());
	AddCommandLineTool(new DumpTool());
    
 
    ParseCommandLine();
    
    DetectCommandLineMode();
    
    if(isEnabled)
    {
        Logger::Instance()->EnableConsoleMode();
    }

    FindActiveTool();
}

CommandLineManager::~CommandLineManager()
{
    Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
    for(auto it = commandLineTools.begin(); it != endIT; ++it)
    {
        SafeDelete(it->second);
    }
    commandLineTools.clear();
}

void CommandLineManager::AddCommandLineTool(CommandLineTool *tool)
{
    DVASSERT(tool);
    
    if(commandLineTools.find(tool->GetCommandLineKey()) != commandLineTools.end())
    {
        SafeDelete(commandLineTools[tool->GetCommandLineKey()]);
    }
    
    commandLineTools[tool->GetCommandLineKey()] = tool;
}

void CommandLineManager::ParseCommandLine()
{
    if(CommandLineParser::CommandIsFound(String("-h")) || CommandLineParser::CommandIsFound(String("--help")))
    {
        PrintUsage();

		isEnabled = true;
        return;
    }
    
    if(CommandLineParser::CommandIsFound(String("-v")) || CommandLineParser::CommandIsFound(String("--verbose")))
    {
        CommandLineParser::Instance()->SetVerbose(true);
    }
    
    if(CommandLineParser::CommandIsFound(String("-exo")))
    {
        CommandLineParser::Instance()->SetExtendedOutput(true);
    }

    if(CommandLineParser::CommandIsFound(String("-teamcity")))
    {
        CommandLineParser::Instance()->SetUseTeamcityOutput(true);
    }

}

void CommandLineManager::DetectCommandLineMode()
{
    isEnabled |= Core::Instance()->IsConsoleMode();
    
    Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
    for(auto it = commandLineTools.begin(); it != endIT; ++it)
    {
        if(CommandLineParser::CommandIsFound(it->first))
        {
            isEnabled = true;
            break;
        }
    }
}


void CommandLineManager::FindActiveTool()
{
    activeTool = NULL;
    if(isEnabled)
    {
        Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
        for(auto it = commandLineTools.begin(); it != endIT; ++it)
        {
            if(CommandLineParser::CommandIsFound(it->first))
            {
                activeTool = it->second;
                break;
            }
        }
    }
}

void CommandLineManager::Process()
{
    if(activeTool)
    {
        const FilePath qualitySettings = activeTool->GetQualityConfigPath();
        if(!qualitySettings.IsEmpty())
        {
            QualitySettingsSystem::Instance()->Load(activeTool->GetQualityConfigPath());
        }

        activeTool->Process();
    }
}

void CommandLineManager::PrintResults()
{
    if(!activeTool) return;
    
    const Set<String> &errors = activeTool->GetErrorList();
    if(0 < errors.size())
    {
        Logger::Error("Errors:");
        Set<String>::const_iterator endIt = errors.end();
        int32 index = 0;
        for (auto it = errors.begin(); it != endIt; ++it)
        {
            Logger::Error(Format("[%d] %s\n", index, (*it).c_str()).c_str());
            
            ++index;
        }
        
        if(isToolInitialized)
            ShowErrorDialog(errors);
    }
}

void CommandLineManager::InitalizeTool()
{
	if(activeTool)
	{
		isToolInitialized = activeTool->InitializeFromCommandLine();
        activeTool->DumpParams();
	}

    if(CommandLineParser::Instance()->UseTeamcityOutput())
    {
        DAVA::TeamcityOutput *out = new DAVA::TeamcityOutput();
        DAVA::Logger::AddCustomOutput(out);
    }
}
