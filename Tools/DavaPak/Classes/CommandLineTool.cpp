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

#include "CommandLineTool.h"
#include "Logger/Logger.h"
#include "Logger/TeamcityOutput.h"
#include "CommandLine/CommandLineParser.h"

using namespace DAVA;

CommandLineTool::CommandLineTool(const DAVA::String& toolName)
    : options(toolName)
{
    options.AddOption("-v", VariantType(false), "Verbose output");
    options.AddOption("-h", VariantType(false), "Help for command");
    options.AddOption("-teamcity", VariantType(false), "Extra output in teamcity format");
}

bool CommandLineTool::ParseOptions(int argc, char* argv[])
{
    return options.Parse(argc, argv);
}

void CommandLineTool::PrintUsage() const
{
    options.PrintUsage();
}

DAVA::String CommandLineTool::GetToolKey() const
{
    return options.GetCommand();
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

    if (ConvertOptionsToParamsInternal())
    {
        ProcessInternal();
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
        CommandLineParser::Instance()->SetVerbose(true);
        Logger::Instance()->SetLogLevel(Logger::LEVEL_DEBUG);
    }

    const bool useTeamcity = options.GetOption("-teamcity").AsBool();
    if (useTeamcity)
    {
        CommandLineParser::Instance()->SetUseTeamcityOutput(true);
        Logger::AddCustomOutput(new TeamcityOutput());
    }
}
