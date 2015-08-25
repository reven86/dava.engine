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
#include "TexturePacker/CommandLineParser.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"

using namespace DAVA;

void DumpTool::PrintUsage() const
{
    printf("\n");
    printf("-dump -links [-indir] [-processfile] [-outfile] [-qualitycfgpath]\n");
    printf("\twill save all pathnames from scene file to out file\n");
    printf("\t-links - will dump pathnames\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-processfile - filename from DataSource/3d/ for saving\n");
    printf("\t-outfile - path to file for dumping of pathnames\n");
	printf("\t-qualitycfgpath - path for quality.yaml file\n");

    printf("\n");
    printf("Samples:\n");
    printf("-dump -links -indir /Users/User/Project/DataSource/3d -processfile Maps/level.sc2 -outfile /Users/User/links.txt \n");
	printf("-dump -links -indir /Users/User/Project/DataSource/3d -processfile Maps/level.sc2 -outfile /Users/User/links.txt -qualitycfgpath /Users/User/quality.yaml\n");
}

DAVA::String DumpTool::GetCommandLineKey() const
{
    return "-dump";
}

bool DumpTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
    inFolder = CommandLineParser::GetCommandParam(String("-indir"));
    if(inFolder.IsEmpty())
    {
        errors.insert("[DumpTool] Incorrect indir parameter");
        return false;
    }
    inFolder.MakeDirectoryPathname();
    
    filename = CommandLineParser::GetCommandParam(String("-processfile"));
    if(filename.empty())
    {
        errors.insert("[DumpTool] Filename is not set");
        return false;
    }
    
	qualityPathname = CommandLineParser::GetCommandParam(String("-qualitycfgpath"));

    if(CommandLineParser::CommandIsFound(String("-links")))
    {
        commandAction = ACTION_DUMP_LINKS;
		outFile = CommandLineParser::GetCommandParam(String("-outfile"));
		if (outFile.IsEmpty())
        {
            errors.insert("[DumpTool] Incorrect outFile parameter");
            return false;
        }
    }
    else
    {
        errors.insert("[SceneSaverTool] Incorrect action");
        return false;
    }
    
    return true;
}

void DumpTool::DumpParams() const
{
	Logger::Info("DumpTool started with params:\n\tIn folder: %s\n\tFilename: %s\n\tOut file: %s\n\tQuality file: %s\n",
		inFolder.GetStringValue().c_str(), filename.c_str(), outFile.GetStringValue().c_str(), qualityPathname.GetStringValue().c_str());
}

void DumpTool::Process() 
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

