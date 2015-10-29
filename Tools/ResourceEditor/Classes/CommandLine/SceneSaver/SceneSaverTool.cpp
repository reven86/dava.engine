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

#include "CommandLine/SceneSaver/SceneSaverTool.h"
#include "CommandLine/SceneSaver/SceneSaver.h"

using namespace DAVA;

namespace OptionName
{
static const String Save = "-save";
static const String Resave = "-resave";
static const String Yaml = "-yaml";
static const String InDir = "-indir";
static const String OutDir = "-outdir";
static const String ProcessFile = "-processfile";
static const String CopyConverted = "-copyconverted";
}

SceneSaverTool::SceneSaverTool()
    : CommandLineTool("-scenesaver")
{
    options.AddOption(OptionName::Save, VariantType(false), "Saving scene from indir to outdir");
    options.AddOption(OptionName::Resave, VariantType(false), "Resave file into indir");
    options.AddOption(OptionName::Yaml, VariantType(false), "Target is *.yaml file");
    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::OutDir, VariantType(String("")), "Path for Project/Data/3d/ folder");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for exporting");
    options.AddOption(OptionName::CopyConverted, VariantType(false), "Enables copying of converted image files");
}

void SceneSaverTool::ConvertOptionsToParamsInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    outFolder = options.GetOption(OptionName::OutDir).AsString();
    filename = options.GetOption(OptionName::ProcessFile).AsString();

    if (options.GetOption(OptionName::Save).AsBool())
    {
        commandAction = ACTION_SAVE;
    }
    else if (options.GetOption(OptionName::Resave).AsBool())
    {
        if (options.GetOption(OptionName::Yaml).AsBool())
        {
            commandAction = ACTION_RESAVE_YAML;
        }
        else
        {
            commandAction = ACTION_RESAVE_SCENE;
        }
    }

    copyConverted = options.GetOption(OptionName::CopyConverted).AsBool();
}

bool SceneSaverTool::InitializeInternal()
{
    if (inFolder.IsEmpty())
    {
        AddError("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

    if (commandAction == ACTION_SAVE)
    {
        if (outFolder.IsEmpty())
        {
            AddError("Output folder was not selected");
            return false;
        }
        outFolder.MakeDirectoryPathname();
    }
    else if (commandAction == ACTION_NONE)
    {
        AddError("Wrong action was selected");
        return false;
    }

    if (filename.empty() && (commandAction != eAction::ACTION_RESAVE_YAML))
    {
        AddError("Filename was not selected");
        return false;
    }
    
    return true;
}

void SceneSaverTool::ProcessInternal()
{
    switch (commandAction)
    {
    case SceneSaverTool::eAction::ACTION_SAVE:
    {
        SceneSaver saver;
        saver.SetInFolder(inFolder);
        saver.SetOutFolder(outFolder);
        saver.EnableCopyConverted(copyConverted);
        saver.SaveFile(filename, errors);

        break;
    }
    case SceneSaverTool::eAction::ACTION_RESAVE_SCENE:
    {
        SceneSaver saver;
        saver.SetInFolder(inFolder);
        saver.ResaveFile(filename, errors);
        break;
    }
    case SceneSaverTool::eAction::ACTION_RESAVE_YAML:
    {
        SceneSaver saver;
        saver.ResaveYamlFilesRecursive(inFolder, errors);
        break;
    }

    default:
        DVASSERT(false);
        break;
    }
}

DAVA::FilePath SceneSaverTool::GetQualityConfigPath() const
{
    return CreateQualityConfigPath(inFolder);
}

