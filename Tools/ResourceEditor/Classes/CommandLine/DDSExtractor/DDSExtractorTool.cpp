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


#include "DDSExtractorTool.h"

#include "TexturePacker/CommandLineParser.h"
#include "Render/Image/LibDdsHelper.h"

void DDSExtractorTool::PrintUsage() const
{
    printf("\n");
    printf("-extract [-path [path to folder or file] -mipmap [number]]\n");
    printf("\tif [number] argument added - one mipmap will be extractacted (counting begins for 1!),\n");
    printf("\totherwise - all present mipmaps will be extracted\n");
    printf("\tfrom specifed file or all dds files, located in specified folder\n");

    printf("\n");
    printf("Sample:\n");
    printf("-extract -path /Users/User/Project/Data/3d -mipmap 2 -forceclose\n");
}

DAVA::String DDSExtractorTool::GetCommandLineKey() const
{
    return "-extract";
}

bool DDSExtractorTool::InitializeFromCommandLine()
{
	sourcePath = DAVA::CommandLineParser::GetCommandParam(DAVA::String("-path"));
	
	if(sourcePath.IsEmpty())
	{
		errors.insert(DAVA::String("Incorrect param for source path"));
		return false;
	}
	
	DAVA::String mipmapNumberStr = DAVA::CommandLineParser::GetCommandParam(DAVA::String("-mipmap"));
	
	mipmapNumber = atoi(mipmapNumberStr.c_str());
	
	return true;
}

void DDSExtractorTool::Process() 
{
	if (DAVA::FileSystem::Instance()->IsFile(sourcePath))
	{
		ExtractImagesFromFile(sourcePath);
		return;
	}
	if (!DAVA::FileSystem::Instance()->IsDirectory(sourcePath))
	{
        errors.insert(DAVA::String("Incorrect param for source path"));
		return;
	}
	sourcePath.MakeDirectoryPathname();
	DAVA::List<DAVA::FilePath> allFiles;
    GetFilesFromFolderRecursively(sourcePath, allFiles);
	DAVA::List<DAVA::FilePath>::const_iterator it = allFiles.begin();
	DAVA::List<DAVA::FilePath>::const_iterator end = allFiles.end();
	for(; it != end; ++it)
	{
		ExtractImagesFromFile(*it);
	}
}

void DDSExtractorTool::ExtractImagesFromFile(const DAVA::FilePath& pathToDDS) 
{
	if(!pathToDDS.IsEqualToExtension(".dds"))
	{
		return;
	}
	
	DAVA::Vector<DAVA::Image *> imageSet;
	//extracted images should have rgba format, but not DX1..DX5, even in case of dxt supporting systems(like windows)
    DAVA::File* file = DAVA::File::Create(pathToDDS, DAVA::File::OPEN | DAVA::File::READ);
    DAVA::int32 mipMapsCount = DAVA::LibDdsHelper::GetMipMapLevelsCount(file);
    DAVA::LibDdsHelper* helper = static_cast<DAVA::LibDdsHelper* >(DAVA::ImageSystem::Instance()->GetImageFormatInterface(DAVA::IMAGE_FORMAT_DDS));

    helper->ReadFile(file, imageSet, mipMapsCount, true);
	SafeRelease(file);
    if(!imageSet.size())
    {
        errors.insert(DAVA::Format("Can not read file %s", pathToDDS.GetAbsolutePathname().c_str()));
        return;
    }
	if (mipmapNumber == 0)
	{
		// if "-mipmap" argumant is blank -> all mipmaps will be extracted
		for (DAVA::uint32 i = 0; i < imageSet.size(); ++i)
		{
			SaveImageAsPNG(pathToDDS, imageSet[i], true);
		}
	}
	else
	{
        if (mipmapNumber <= imageSet.size())
        {
            SaveImageAsPNG(pathToDDS, imageSet[mipmapNumber - 1], false);
        }
        else
        {
            errors.insert(DAVA::Format("Incorrect mipmap number argument: %d, size of mipmaps set in file %s : %d",
                                       mipmapNumber, pathToDDS.GetAbsolutePathname().c_str(), imageSet.size()));
        }
    }
    for_each(imageSet.begin(), imageSet.end(), DAVA::SafeRelease<DAVA::Image>);
}

void DDSExtractorTool::SaveImageAsPNG(const DAVA::FilePath& pathToDDS, DAVA::Image* imageToSave, bool addHeightIntoName)
{
	DAVA::FilePath saveFilePath(pathToDDS);
	saveFilePath.ReplaceExtension(".png");
	if(addHeightIntoName)
	{
		saveFilePath.ReplaceBasename(pathToDDS.GetBasename() + DAVA::Format("_%u", imageToSave->GetHeight()));
	}
	
    DAVA::ImageSystem::Instance()->Save(saveFilePath, imageToSave);
	printf("\n");
	printf(DAVA::Format("Converted: %s", saveFilePath.GetAbsolutePathname().c_str()).c_str());
}

void DDSExtractorTool::GetFilesFromFolderRecursively(const DAVA::FilePath& path, DAVA::List<DAVA::FilePath>& outputList)
{
	DAVA::FileList * fileList = new DAVA::FileList(path);
	for(DAVA::int32 i = 0; i < fileList->GetCount(); ++i)
	{
		if (fileList->IsNavigationDirectory(i))
		{
			continue;
		}
		if(fileList->IsDirectory(i))
		{
            GetFilesFromFolderRecursively(fileList->GetPathname(i), outputList);
		}
		else
		{
			outputList.push_back(fileList->GetPathname(i));
		}
	}
	SafeRelease(fileList);
}

