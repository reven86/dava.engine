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


#include "SceneSaverTool.h"
#include "SceneSaver.h"

#include "TexturePacker/CommandLineParser.h"

using namespace DAVA;

void SceneSaverTool::PrintUsage() const
{
    printf("\n");
    printf("-scenesaver -save [-indir [directory]] [-outdir [directory]] [-processfile [directory]] [-copyconverted]\n");
    printf("-scenesaver -resave [-indir [directory]] [-processfile [directory]] [-forceclose] [-copyconverted]\n");
    printf("\twill save scene file from DataSource/3d to any Data or DataSource folder\n");
    printf("\t-save - will save level to selected Data/3d/\n");
    printf("\t-resave - will open and save level\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-outdir - path for Poject/Data/3d/ folder\n");
    printf("\t-processfile - filename from DataSource/3d/ for saving\n");
    printf("\t-copyconverted - copy *.pvr and *.dds files too\n");

    printf("\n");
    printf("Samples:\n");
    printf("-scenesaver -save -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -forceclose -copyconverted\n");
    printf("-scenesaver -resave -indir /Users/User/Project/DataSource/3d -processfile Maps/level.sc2 -forceclose\n");
}

DAVA::String SceneSaverTool::GetCommandLineKey() const
{
    return "-scenesaver";
}

bool SceneSaverTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
    inFolder = CommandLineParser::GetCommandParam(String("-indir"));
    if(inFolder.IsEmpty())
    {
        errors.insert("[SceneSaverTool] Incorrect indir parameter");
        return false;
    }
    inFolder.MakeDirectoryPathname();
    
    filename = CommandLineParser::GetCommandParam(String("-processfile"));
    if(filename.empty())
    {
        errors.insert("[SceneSaverTool] Filename is not set");
        return false;
    }
    
    if(CommandLineParser::CommandIsFound(String("-save")))
    {
        commandAction = ACTION_SAVE;
        outFolder = CommandLineParser::GetCommandParam(String("-outdir"));
        if(outFolder.IsEmpty())
        {
            errors.insert("[SceneSaverTool] Incorrect outdir parameter");
            return false;
        }
        outFolder.MakeDirectoryPathname();

        copyConverted = CommandLineParser::CommandIsFound(String("-copyconverted"));
    }
    else if(CommandLineParser::CommandIsFound(String("-resave")))
    {
        commandAction = ACTION_RESAVE;
    }
    else
    {
        errors.insert("[SceneSaverTool] Incorrect action");
        return false;
    }
    
    return true;
}

void SceneSaverTool::DumpParams() const
{
    Logger::Info("SceneSaver started with params:\n\tIn folder: %s\n\tOut folder: %s\n\tFilename: %s\n\tCopy converted: %d", inFolder.GetStringValue().c_str(), outFolder.GetStringValue().c_str(),filename.c_str(), copyConverted);
}

void SceneSaverTool::Process() 
{
    SceneSaver saver;

    saver.SetInFolder(inFolder);
    if(commandAction == ACTION_SAVE)
    {
        saver.SetOutFolder(outFolder);
        saver.EnableCopyConverted(copyConverted);
        saver.SaveFile(filename, errors);
    }
    else if(commandAction == ACTION_RESAVE)
    {
        saver.ResaveFile(filename, errors);
    }
}

DAVA::FilePath SceneSaverTool::GetQualityConfigPath() const
{
    return CreateQualityConfigPath(inFolder);
}

