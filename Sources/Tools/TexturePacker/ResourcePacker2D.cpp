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


#include "TexturePacker/ResourcePacker2D.h"
#include "TexturePacker/DefinitionFile.h"
#include "TexturePacker/TexturePacker.h"
#include "TexturePacker/CommandLineParser.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Core/Core.h"
#include "Platform/SystemTimer.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"

#include "Render/GPUFamilyDescriptor.h"
#include "FramePathHelper.h"

#include "IMagickHelper.h"

#include "AssetCache/AssetCache.h"
#include "Platform/Process.h"


namespace DAVA
{

const String ResourcePacker2D::VERSION = "0.0.1";
    
    
static const String FLAG_RECURSIVE = "--recursive";

ResourcePacker2D::ResourcePacker2D()
{
	isLightmapsPacking = false;
	clearProcessDirectory = false;
}

String ResourcePacker2D::GetProcessFolderName()
{
	return "$process/";
}

void ResourcePacker2D::InitFolders(const FilePath & inputPath,const FilePath & outputPath)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());
    
	inputGfxDirectory = inputPath;
	outputGfxDirectory = outputPath;
	excludeDirectory = inputPath + "../";
}
    
void ResourcePacker2D::PackResources(eGPUFamily forGPU)
{
	Logger::FrameworkDebug("\nInput: %s \nOutput: %s \nExclude: %s",
                  inputGfxDirectory.GetAbsolutePathname().c_str(),
                  outputGfxDirectory.GetAbsolutePathname().c_str(),
                  excludeDirectory.GetAbsolutePathname().c_str());

    Logger::FrameworkDebug("For GPU: %s", (GPU_INVALID != forGPU) ? GPUFamilyDescriptor::GetGPUName(forGPU).c_str() : "Unknown");

    
	requestedGPUFamily = forGPU;
    
	isGfxModified = false;

    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
	std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);


	FilePath processDirectoryPath = excludeDirectory + GetProcessFolderName();
	if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
	}


	if (IsMD5ChangedDir(processDirectoryPath, outputGfxDirectory, gfxDirName + ".md5", true))
	{
		if (Core::Instance()->IsConsoleMode())
			Logger::FrameworkDebug("[Gfx not available or changed - performing full repack]");
		isGfxModified = true;
	
		// Remove whole output directory
		bool result = FileSystem::Instance()->DeleteDirectory(outputGfxDirectory);
		if (result)
		{
			Logger::FrameworkDebug("Removed output directory: %s", outputGfxDirectory.GetAbsolutePathname().c_str());
		}
		if (!result && Core::Instance()->IsConsoleMode() && CommandLineParser::Instance()->GetVerbose())
		{
			AddError(Format("[ERROR: Can't delete directory %s]",
									outputGfxDirectory.GetAbsolutePathname().c_str()));
		}
	}

	RecursiveTreeWalk(inputGfxDirectory, outputGfxDirectory);

	// Put latest md5 after convertation
	IsMD5ChangedDir(processDirectoryPath, outputGfxDirectory, gfxDirName + ".md5", true);
}

void ResourcePacker2D::RecalculateMD5ForOutputDir()
{
    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
    std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);

    FilePath processDirectoryPath = excludeDirectory + GetProcessFolderName();
    FileSystem::Instance()->CreateDirectory(processDirectoryPath, true);

    IsMD5ChangedDir(processDirectoryPath, outputGfxDirectory, gfxDirName + ".md5", true);
}


bool ResourcePacker2D::IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & name, bool isRecursive)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());

	FilePath md5FileName = FilePath::CreateWithNewExtension(processDirectoryPath + name, ".md5");

    
	uint8 oldMD5Digest[16];
	uint8 newMD5Digest[16];
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);
	if (!file)
	{
		isChanged = true;		
	}else
	{
		int32 bytes = file->Read(oldMD5Digest, 16);
		DVASSERT(bytes == 16 && "We should always read 16 bytes from md5 file");
	}
	SafeRelease(file);

    MD5::ForDirectory(pathname, newMD5Digest, isRecursive, /*includeHidden=*/false);

	file = File::Create(md5FileName, File::CREATE | File::WRITE);
    DVASSERT(file);
    
	int32 bytes = file->Write(newMD5Digest, 16);
	DVASSERT(bytes == 16 && "16 bytes should be always written for md5 file");
	SafeRelease(file);

	// if already changed return without compare
	if (isChanged)
		return true;

	for (int32 k = 0; k < 16; ++k)
		if (oldMD5Digest[k] != newMD5Digest[k])
			isChanged = true;

	return isChanged;
}


bool ResourcePacker2D::IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());

	FilePath md5FileName = FilePath::CreateWithNewExtension(processDirectoryPath + psdName, ".md5");

	std::array<uint8, MD5::DIGEST_SIZE> oldMD5Digest;
	std::array<uint8, MD5::DIGEST_SIZE> newMD5Digest;
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);

	if (nullptr == file)
		isChanged = true;		
	else
	{
		int32 bytes = file->Read(oldMD5Digest.data(), MD5::DIGEST_SIZE);
		DVASSERT(bytes == MD5::DIGEST_SIZE && "We should always read 16 bytes from md5 file");
		SafeRelease(file);
	}
		
	MD5::ForFile(pathname, newMD5Digest.data());
	
	file = File::Create(md5FileName, File::CREATE | File::WRITE);
	int32 bytes = file->Write(newMD5Digest.data(), MD5::DIGEST_SIZE);
	DVASSERT(bytes == MD5::DIGEST_SIZE && "16 bytes should be always written for md5 file");
	SafeRelease(file);

	return isChanged ? true : oldMD5Digest != newMD5Digest;
}

DefinitionFile * ResourcePacker2D::ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName, bool twoSideMargin, uint32 texturesMargin)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());
    
	uint32 maxTextureSize = (CommandLineParser::Instance()->IsFlagSet("--tsize4096")) ? TexturePacker::TSIZE_4096 : TexturePacker::DEFAULT_TEXTURE_SIZE;

	bool withAlpha = CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha");
    bool useLayerNames = CommandLineParser::Instance()->IsFlagSet("--useLayerNames");
	
    FilePath psdNameWithoutExtension(processDirectoryPath + psdName);
    psdNameWithoutExtension.TruncateExtension();
	
    IMagickHelper::CroppedData cropped_data;
    
    IMagickHelper::ConvertToPNGCroppedGeometry( psdPathname.GetAbsolutePathname().c_str(), processDirectoryPath.GetAbsolutePathname().c_str() , &cropped_data, true );
		
	if ( cropped_data.layers_array_size == 0 )
	{
		AddError(Format("Number of layers is too low: %s", psdPathname.GetAbsolutePathname().c_str()));
		return 0;
	}
		
	//Logger::FrameworkDebug("psd file: %s wext: %s", psdPathname.c_str(), psdNameWithoutExtension.c_str());
		
	int width  = cropped_data.layer_width;
	int height = cropped_data.layer_height;
		
	DefinitionFile * defFile = new DefinitionFile;
	defFile->filename = psdNameWithoutExtension + ".txt";

	defFile->spriteWidth = width;
	defFile->spriteHeight = height;
	defFile->frameCount = (int)cropped_data.layers_array_size -1;
	defFile->frameRects = new Rect2i[defFile->frameCount];

	for(int k = 1; k < (int)cropped_data.layers_array_size; ++k)
	{
		//save layer names
        String layerName;
        
        if (useLayerNames)
        {
            layerName.assign(cropped_data.layers_array[k].name);
            if (layerName.empty())
            {
                Logger::Warning("* WARNING * - %s layer %d has empty name!!!", psdName.c_str(), k - 1);
            }
            // Check if layer name is unique
            Vector<String>::iterator it = find(defFile->frameNames.begin(), defFile->frameNames.end(), layerName);
            if (it != defFile->frameNames.end())
            {
                Logger::Warning("* WARNING * - %s layer %d name %s is not unique!!!", psdName.c_str(), k - 1, layerName.c_str());
            }
        }
        else
        {
            layerName.assign("frame");
            layerName.append(std::to_string(k - 1));
        }
        
		defFile->frameNames.push_back(layerName);


		//save layer rects
		if ( !withAlpha )
		{
			defFile->frameRects[k - 1] = Rect2i(cropped_data.layers_array[k].x, cropped_data.layers_array[k].y, cropped_data.layers_array[k].dx, cropped_data.layers_array[k].dy) ;

			//printf("Percent: %d Aspect: %d Greater: %d Less: %d\n", (int)bbox.percent(), (int)bbox.aspect(), (int)bbox.greater(), (int)bbox.less());

			if ((defFile->frameRects[k - 1].dx > (int32)maxTextureSize) || (defFile->frameRects[k - 1].dy > (int32)maxTextureSize))
			{
				Logger::Warning("* WARNING * - frame of %s layer %d is bigger than maxTextureSize(%d) layer exportSize (%d x %d) FORCE REDUCE TO (%d x %d). Bewarned!!! Results not guaranteed!!!", psdName.c_str(), k - 1, maxTextureSize
					, defFile->frameRects[k - 1].dx, defFile->frameRects[k - 1].dy, width, height);

				defFile->frameRects[k - 1].dx = width;
				defFile->frameRects[k - 1].dy = height;
			}
			else
			{
				if ((defFile->frameRects[k - 1].dx > width))
				{
					Logger::Warning("For texture %s, layer %d width is bigger than sprite width: %d > %d. Layer width will be reduced to the sprite value", psdName.c_str(), k - 1, defFile->frameRects[k - 1].dx, width);
					defFile->frameRects[k - 1].dx = width;
				}

				if ((defFile->frameRects[k - 1].dy > height))
				{
					Logger::Warning("For texture %s, layer %d height is bigger than sprite height: %d > %d. Layer height will be reduced to the sprite value", psdName.c_str(), k - 1, defFile->frameRects[k - 1].dy, height);
					defFile->frameRects[k - 1].dy = height;
				}
			}
		}
		else
			defFile->frameRects[k - 1] = Rect2i(cropped_data.layers_array[k].x, cropped_data.layers_array[k].y, width, height);

		// add borders
		if (!twoSideMargin )
		{
			defFile->frameRects[k - 1].dx += texturesMargin;
			defFile->frameRects[k - 1].dy += texturesMargin;
		}
	}
		
	return defFile;

}

Vector<String> ResourcePacker2D::FetchFlags(const FilePath & flagsPathname)
{
    Vector<String> tokens;

	File * file = File::Create(flagsPathname, File::READ | File::OPEN);
	if (!file)
	{
		AddError(Format("Failed to open file: %s", flagsPathname.GetAbsolutePathname().c_str()));
		
        return tokens;
	}

	String tokenString = file->ReadLine();
    Split(tokenString, " ", tokens, false);

    for (auto& token : tokens)
    {
        Logger::FrameworkDebug("Token: %s", token.c_str());
    }

	SafeRelease(file);
	
	return tokens;
}


bool ResourcePacker2D::isRecursiveFlagSet(const Vector<String> & flags)
{
	for (uint32 k = 0; k < flags.size(); ++k)
	{
		if (flags[k] == FLAG_RECURSIVE)
		{
			return true;
		}
	}
	
	return false;
}

void ResourcePacker2D::RecursiveTreeWalk(const FilePath & inputPath, const FilePath & outputPath, const Vector<String> & passedFlags)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());

	String inputRelativePath = inputPath.GetRelativePathname(excludeDirectory);
	FilePath processDirectoryPath = excludeDirectory  + GetProcessFolderName() + inputRelativePath;
    FileSystem::Instance()->CreateDirectory(processDirectoryPath, true);

	if(clearProcessDirectory)
	{
		FileSystem::Instance()->DeleteDirectoryFiles(processDirectoryPath, false);
	}

    FileSystem::Instance()->CreateDirectory(outputPath);
	
    Vector<String> currentCommandFlags;
    List<DefinitionFile *> definitionFileList;

    const auto flagsPathname = inputPath + "flags.txt";
    if(flagsPathname.Exists())
    {
        currentCommandFlags = FetchFlags(flagsPathname);
    }
    else
    {
        currentCommandFlags = passedFlags;
    }
    

    CommandLineParser::Instance()->SetArguments(currentCommandFlags);

	
	bool modified = isGfxModified;
	// Process all psd / png files

    if (IsMD5ChangedDir(processDirectoryPath, inputPath, "dir.md5", false) || 
        CommandLineParser::CommandIsFound(String("-forceModify")))
    {
        modified = true;
    }
    //TODO:AC: hash of folder is in dir.md5 file, see lines above
    //TODO:AC:
    /*
        we need ask cache server about files from this cache and currentCommandFlags + command line arguments
    */
    AssetCache::CacheItemKey cacheKey;
    {   //Detect cache key
        auto md5FileName = FilePath::CreateWithNewExtension(processDirectoryPath + "dir.md5", ".md5");
        ScopedPtr<File> md5File(File::Create(md5FileName, File::OPEN | File::READ));
        if(static_cast<File *>(md5File) != nullptr)
        {   //invalidation of primary key
            auto read = md5File->Read(cacheKey.keyData.hash.primary, MD5::DIGEST_SIZE);
            DVASSERT(read == MD5::DIGEST_SIZE);
        }
        
        String cachedParams;
        Merge(currentCommandFlags, ',', cachedParams);
        
        cachedParams += String("GPU = ") + GPUFamilyDescriptor::GetGPUName(requestedGPUFamily);
        cachedParams += String("PackerVersion = ") + VERSION;
        
        auto strDataPtr = cachedParams.c_str();
        auto strDataSize = cachedParams.size();
        MD5::ForData(reinterpret_cast<const uint8 *>(strDataPtr), static_cast<uint32>(strDataSize), cacheKey.keyData.hash.secondary);
    }
    
    bool skipRepackOfFolder = !modified;
    if(modified)
    {
        uint64 getTime = SystemTimer::Instance()->AbsoluteMS();
        skipRepackOfFolder = GetFilesFromCache(cacheKey, outputPath);
        getTime = SystemTimer::Instance()->AbsoluteMS() - getTime;
        Logger::Info("[%s - %.2lf secs] - GET FROM CACHE", inputPath.GetAbsolutePathname().c_str(), (float64)(getTime) / 1000.0f);
    }

    //TODO:AC: end
    
    ScopedPtr<FileList> fileList(new FileList(inputPath));
    fileList->Sort();
    if(!skipRepackOfFolder)
    {
        uint64 packTime = SystemTimer::Instance()->AbsoluteMS();

        // read textures margins settings
        bool useTwoSideMargin = CommandLineParser::Instance()->IsFlagSet("--add2sidepixel");
        uint32 marginInPixels = TexturePacker::DEFAULT_MARGIN;
        if (!useTwoSideMargin)
        {
            if (CommandLineParser::Instance()->IsFlagSet("--add0pixel"))
                marginInPixels = 0;
            else if (CommandLineParser::Instance()->IsFlagSet("--add1pixel"))
                marginInPixels = 1;
            else if (CommandLineParser::Instance()->IsFlagSet("--add2pixel"))
                marginInPixels = 2;
            else if (CommandLineParser::Instance()->IsFlagSet("--add4pixel"))
                marginInPixels = 4;
        }
        
        
        bool needPackResourcesInThisDir = true;
        if (modified)
        {
            FileSystem::Instance()->DeleteDirectoryFiles(outputPath, false);
            
            for (int fi = 0; fi < fileList->GetCount(); ++fi)
            {
                if (!fileList->IsDirectory(fi))
                {
                    FilePath fullname = fileList->GetPathname(fi);
                    if (fullname.IsEqualToExtension(".psd"))
                    {
                        //TODO: check if we need filename or pathname
                        DefinitionFile * defFile = ProcessPSD(processDirectoryPath, fullname, fullname.GetFilename(), useTwoSideMargin, marginInPixels);
                        if (!defFile)
                        {
                            // An error occured while converting this PSD file - cancel converting in this directory.
                            needPackResourcesInThisDir = false;
                            break;
                        }
                        
                        definitionFileList.push_back(defFile);
                    }
                    else if(isLightmapsPacking && fullname.IsEqualToExtension(".png"))
                    {
                        DefinitionFile * defFile = new DefinitionFile();
                        defFile->LoadPNG(fullname, processDirectoryPath);
                        definitionFileList.push_back(defFile);
                    }
                    else if (fullname.IsEqualToExtension(".pngdef"))
                    {
                        DefinitionFile * defFile = new DefinitionFile();
                        if (defFile->LoadPNGDef(fullname, processDirectoryPath, useTwoSideMargin, marginInPixels))
                        {
                            definitionFileList.push_back(defFile);
                        }
                        else
                        {
                            SafeDelete(defFile);
                        }
                    }
                }
            }
            
            if (modified && !definitionFileList.empty())
            {
                TexturePacker packer;
                if(isLightmapsPacking)
                {
                    packer.UseOnlySquareTextures();
                    packer.SetMaxTextureSize(2048);
                }
                else if(CommandLineParser::Instance()->IsFlagSet("--tsize4096"))
                {
                    packer.SetMaxTextureSize(TexturePacker::TSIZE_4096);
                }
                
                packer.SetTwoSideMargin(useTwoSideMargin);
                packer.SetTexturesMargin(marginInPixels);
                
                if (CommandLineParser::Instance()->IsFlagSet("--split"))
                {
                    packer.PackToTexturesSeparate(excludeDirectory, outputPath, definitionFileList, requestedGPUFamily);
                }
                else
                {
                    packer.PackToTextures(excludeDirectory, outputPath, definitionFileList, requestedGPUFamily);
                }
                
                Set<String> currentErrors = packer.GetErrors();
                if (!currentErrors.empty())
                {
                    errors.insert(currentErrors.begin(), currentErrors.end());
                }
            }
        }
        
        packTime = SystemTimer::Instance()->AbsoluteMS() - packTime;
        
        if (Core::Instance()->IsConsoleMode())
        {
            String flagsString;
            Merge(currentCommandFlags, ' ', flagsString);
            Logger::Info("[%d files packed with flags: %s]", (int)definitionFileList.size(), flagsString.c_str());
            
        }

        const char* result = (modified && !definitionFileList.empty()) ? "[REPACKED]" : "[unchanged]";
        Logger::Info("[%s - %.2lf secs] - %s", inputPath.GetAbsolutePathname().c_str(), (float64)packTime / 1000.0f, result);

        for (List<DefinitionFile*>::iterator it = definitionFileList.begin(); it != definitionFileList.end(); ++it)
        {
            DefinitionFile * file = *it;
            SafeDelete(file);
        }
        definitionFileList.clear();
        
        
        
        auto addTime = SystemTimer::Instance()->AbsoluteMS();
        AddFilesToCache(cacheKey, outputPath);
        addTime = SystemTimer::Instance()->AbsoluteMS() - addTime;
        Logger::Info("[%s - %.2lf secs] - AddToCache", inputPath.GetAbsolutePathname().c_str(), (float64)addTime / 1000.0f);
    }
    
	//recursivity
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
			String filename = fileList->GetFilename(fi);
			if (!fileList->IsNavigationDirectory(fi) && (filename != "$process") && (filename != ".svn"))
			{
				if ((filename.size() > 0) && (filename[0] != '.'))
                {
                    FilePath input = inputPath + filename;
                    input.MakeDirectoryPathname();
                    
                    FilePath output = outputPath + filename;
                    output.MakeDirectoryPathname();
					
					if (isRecursiveFlagSet(currentCommandFlags))
					{
						RecursiveTreeWalk(input, output, currentCommandFlags);
					}
					else
					{
						RecursiveTreeWalk(input, output);
					}
                }
			}
		}
	}
}

bool ResourcePacker2D::GetFilesFromCache(const AssetCache::CacheItemKey &key, const FilePath & outputPath)
{
    auto oldDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    FileSystem::Instance()->SetCurrentWorkingDirectory(cacheClientTool.GetDirectory());
    SCOPE_EXIT
    {
        FileSystem::Instance()->SetCurrentWorkingDirectory(oldDir);
    };

    Vector<String> arruments;
    arruments.push_back("get");
    arruments.push_back("-h");
    arruments.push_back(key.ToString());
    
    arruments.push_back("-f");
    arruments.push_back(outputPath.GetAbsolutePathname());
    
    Process cacheClient(cacheClientTool, arruments);
    if(cacheClient.Run(false))
    {
        cacheClient.Wait();
        
        auto exitCode = cacheClient.GetExitCode();
        if(exitCode != 0)
        {
            const String& procOutput = cacheClient.GetOutput();
            if(procOutput.size() > 0)
            {
                Logger::FrameworkDebug("\nCacheClientLog: %s", procOutput.c_str());
            }
        }
        
        return (exitCode == 0);
    }

    
    return false;
}

bool ResourcePacker2D::AddFilesToCache(const AssetCache::CacheItemKey &key, const FilePath & outputPath)
{
    auto oldDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    FileSystem::Instance()->SetCurrentWorkingDirectory(cacheClientTool.GetDirectory());
    SCOPE_EXIT
    {
        FileSystem::Instance()->SetCurrentWorkingDirectory(oldDir);
    };
    
    
    String fileListString;
    ScopedPtr<FileList> outFilesList(new FileList(outputPath));
    for (int fi = 0; fi < outFilesList->GetCount(); ++fi)
    {
        if (!outFilesList->IsDirectory(fi))
        {
            if(fileListString.empty() == false)
            {
                fileListString += String(",");
            }
            
            fileListString += outFilesList->GetPathname(fi).GetAbsolutePathname();
        }
    }
    
    if(fileListString.empty() == false)
    {
        Vector<String> arruments;
        arruments.push_back("add");
        arruments.push_back("-h");
        arruments.push_back(key.ToString());
        arruments.push_back("-f");
        arruments.push_back(fileListString);
        
        if(outFilesList->GetFileCount() > 20)
        {
            arruments.push_back("-t");
            arruments.push_back("5");   //enlarge default timeout
        }
        
        Process cacheClient(cacheClientTool, arruments);
        if(cacheClient.Run(false))
        {
            cacheClient.Wait();
            
            auto exitCode = cacheClient.GetExitCode();
            if(exitCode != 0)
            {
                const String& procOutput = cacheClient.GetOutput();
                if(procOutput.size() > 0)
                {
                    Logger::FrameworkDebug("\nCacheClientLog: %s", procOutput.c_str());
                }
            }
            
            return (exitCode == 0);
        }
    }
    
    return false;
}
    
const Set<String>& ResourcePacker2D::GetErrors() const
{
	return errors;
}

void ResourcePacker2D::AddError(const String& errorMsg)
{
	Logger::Error(errorMsg.c_str());
	errors.insert(errorMsg);
}
    
void ResourcePacker2D::SetCacheClientTool(const DAVA::FilePath &path)
{
    cacheClientTool = path;
}

};
