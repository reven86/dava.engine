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


#include "CommandLine/Dump/DumpTool.h"
#include "CommandLine/Dump/SceneDumper.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"

using namespace DAVA;

namespace OptionName
{
static const String Links = "-links";
static const String InDir = "-indir";
static const String ProcessFile = "-processfile";
static const String QualityConfig = "-qualitycfgpath";
static const String OutFile = "-outfile";
}

DumpTool::DumpTool()
    : CommandLineTool("-dump")
{
    options.AddOption(OptionName::Links, VariantType(false), "Target for dumping is links");
    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for dumping");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
    options.AddOption(OptionName::OutFile, VariantType(String("")), "Full path to file to write result of dumping");
}

void DumpTool::ConvertOptionsToParamsInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    filename = options.GetOption(OptionName::ProcessFile).AsString();
    qualityPathname = options.GetOption(OptionName::QualityConfig).AsString();
    outFile = options.GetOption(OptionName::OutFile).AsString();

    if (options.GetOption(OptionName::Links).AsBool())
    {
        commandAction = ACTION_DUMP_LINKS;
    }
}

bool DumpTool::InitializeInternal()
{
    if(inFolder.IsEmpty())
    {
        AddError("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();
    
    if(filename.empty())
    {
        AddError("Filename was not selected");
        return false;
    }

    if (outFile.IsEmpty())
    {
        AddError("Out file was not selected");
        return false;
    }

    if (commandAction == ACTION_NONE)
    {
        AddError("Target for dumping was not selected");
        return false;
    }

    return true;
}

void DumpTool::ProcessInternal()
{
    if(commandAction == ACTION_DUMP_LINKS)
    {
		auto links = SceneDumper::DumpLinks(inFolder + filename, errors);

        FileSystem::Instance()->CreateDirectory(outFile.GetDirectory(), true);
		ScopedPtr<File> file(File::Create(outFile, File::WRITE | File::CREATE));
		if (file)
		{
			for (const auto &link : links)
			{
				if (!link.IsEmpty() && link.GetType() != FilePath::PATH_IN_MEMORY)
				{
					file->WriteLine(link.GetAbsolutePathname());
				}
			}
		}
    }
}

DAVA::FilePath DumpTool::GetQualityConfigPath() const
{
	if (qualityPathname.IsEmpty())
	{
		return CreateQualityConfigPath(inFolder);
	}

	return qualityPathname;
}

