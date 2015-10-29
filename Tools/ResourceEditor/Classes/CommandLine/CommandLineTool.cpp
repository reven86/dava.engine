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

#include "CommandLine/CommandLineTool.h"
#include "CommandLine/CommandLineParser.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "TeamcityOutput/TeamcityOutput.h"

#include "Project/ProjectManager.h"

using namespace DAVA;

CommandLineTool::CommandLineTool(const String& toolName)
    : options(toolName)
{
    options.AddOption("-v", VariantType(false), "Verbose output");
    options.AddOption("-h", VariantType(false), "Help for command");
    options.AddOption("-teamcity", VariantType(false), "Extra output in teamcity format");
}

DAVA::String CommandLineTool::GetToolKey() const
{
    return options.GetCommand();
}

FilePath CommandLineTool::CreateQualityConfigPath(const FilePath& path) const
{
    FilePath projectPath = ProjectManager::CreateProjectPathFromPath(path);
    if(projectPath.IsEmpty())
        return projectPath;
    
    return (projectPath + "Data/Quality.yaml");
}

bool CommandLineTool::ParseCommandLine(int argc, char* argv[])
{
    return options.Parse(argc, argv);
}

bool CommandLineTool::Initialize()
{
    ConvertOptionsToParamsInternal();
    return InitializeInternal();
}

void CommandLineTool::PrintUsage() const
{
    options.PrintUsage();
}

void CommandLineTool::PrintResults() const
{
    if (!errors.empty())
    {
        Logger::Error("Errors count is %d:", errors.size());

        int32 errorIndex = 0;
        for (auto& error : errors)
        {
            Logger::Error("[%d] %s", errorIndex++, error.c_str());
        }
    }
}

void CommandLineTool::Process()
{
    const bool printUsage = options.GetOption("-h").AsBool();
    if (printUsage)
    {
        PrintUsage();
        return;
    }

    PrepareEnvironment();

    bool initialized = Initialize();
    if (initialized)
    {
        PrepareQualitySystem();
        ProcessInternal();
        PrintResults();
    }
    else
    {
        PrintUsage();
    }
}

void CommandLineTool::PrepareEnvironment() const
{
    const bool verboseMode = options.GetOption("-v").AsBool();
    if (verboseMode)
    {
        CommandLineParser::Instance()->SetVerbose(true); //why we have this function?
        Logger::Instance()->SetLogLevel(Logger::LEVEL_DEBUG);
    }

    const bool useTeamcity = options.GetOption("-teamcity").AsBool();
    if (useTeamcity)
    {
        CommandLineParser::Instance()->SetUseTeamcityOutput(true); //why we have this ?
        Logger::AddCustomOutput(new TeamcityOutput());
    }
}

DAVA::FilePath CommandLineTool::GetQualityConfigPath() const
{
    return DAVA::FilePath();
};

void CommandLineTool::AddError(const DAVA::String& errorMessage)
{
    errors.insert(errorMessage);
    Logger::Error(errorMessage.c_str());
}

void CommandLineTool::PrepareQualitySystem() const
{
    const FilePath qualitySettings = GetQualityConfigPath();
    if (!qualitySettings.IsEmpty())
    {
        QualitySettingsSystem::Instance()->Load(GetQualityConfigPath());
    }
}
