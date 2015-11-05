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
#include "CommandLine/CommandLineParser.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Core/Core.h"
#include "Platform/SystemTimer.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"

#include "Render/GPUFamilyDescriptor.h"

#include "IMagickHelper.h"

#include "AssetCache/AssetCache.h"
#include "Platform/Process.h"

namespace DAVA
{
const String ResourcePacker2D::VERSION = "0.0.1";


enum AssetClientCode : int
{
    OK = 0,
    WRONG_COMMAND_LINE = 1,
    WRONG_IP = 2,
    TIMEOUT = 3,
    CANNOT_CONNECT = 4,
    SERVER_ERROR = 5,
    CANNOT_READ_FILES = 6
};

const String& GetCodeAsString(AssetClientCode code)
{
    static Array<String, 7> codeStrings = { { "OK",
                                              "WRONG_COMMAND_LINE",
                                              "WRONG_IP",
                                              "TIMEOUT",
                                              "CANNOT_CONNECT",
                                              "SERVER_ERROR",
                                              "CANNOT_READ_FILES" } };
    static String codeUnknown("CODE_UNKNOWN");

    if (code >= 0 && code < static_cast<int>(codeStrings.size()))
    {
        return codeStrings[code];
    }
    else
    {
        return codeUnknown;
    }
}

String ResourcePacker2D::GetProcessFolderName()
{
    return "$process/";
}

void ResourcePacker2D::SetConvertQuality(const TextureConverter::eConvertQuality arg)
{
    quality = arg;
}

void ResourcePacker2D::SetRunning(bool arg)
{
    if (arg != running)
    {
        Logger::FrameworkDebug(arg ? "ResourcePacker2D was started" : "ResourcePacker2D was stopped");
    }
    running = arg;
}
void ResourcePacker2D::InitFolders(const FilePath & inputPath,const FilePath & outputPath)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());
    
    inputGfxDirectory = inputPath;
    outputGfxDirectory = outputPath;
    rootDirectory = inputPath + "../";
}
    
void ResourcePacker2D::PackResources(eGPUFamily forGPU)
{
    SetRunning(true);
    Logger::FrameworkDebug("\nInput: %s \nOutput: %s \nExclude: %s",
                           inputGfxDirectory.GetAbsolutePathname().c_str(),
                           outputGfxDirectory.GetAbsolutePathname().c_str(),
                           rootDirectory.GetAbsolutePathname().c_str());

    Logger::FrameworkDebug("For GPU: %s", (GPU_INVALID != forGPU) ? GPUFamilyDescriptor::GetGPUName(forGPU).c_str() : "Unknown");

    
    requestedGPUFamily = forGPU;

    outputDirModified = false;

    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
    std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);

    FilePath processDirectoryPath = rootDirectory + GetProcessFolderName();
    if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
    {
    	//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
    }

    if (RecalculateDirMD5(outputGfxDirectory, processDirectoryPath + gfxDirName + ".md5", true))
    {
        if (Core::Instance()->IsConsoleMode())
        {
            Logger::FrameworkDebug("[Gfx not available or changed - performing full repack]");
        }
        outputDirModified = true;

        // Remove whole output directory
        if (clearOutputDirectory)
        {
            bool isDeleted = FileSystem::Instance()->DeleteDirectory(outputGfxDirectory);
            if (isDeleted)
            {
                Logger::FrameworkDebug("Removed output directory: %s", outputGfxDirectory.GetAbsolutePathname().c_str());
            }
            else
            {
                if (FileSystem::Instance()->IsDirectory(outputGfxDirectory))
                {
                    AddError(Format("Can't delete directory [%s]", outputGfxDirectory.GetAbsolutePathname().c_str()));
                }
            }
        }
    }

    RecursiveTreeWalk(inputGfxDirectory, outputGfxDirectory);

    // Put latest md5 after convertation
    RecalculateDirMD5(outputGfxDirectory, processDirectoryPath + gfxDirName + ".md5", true);
}

void ResourcePacker2D::RecalculateMD5ForOutputDir()
{
    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
    std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);

    FilePath processDirectoryPath = rootDirectory + GetProcessFolderName();
    FileSystem::Instance()->CreateDirectory(processDirectoryPath, true);

    RecalculateDirMD5(outputGfxDirectory, processDirectoryPath + gfxDirName + ".md5", true);
}

bool ResourcePacker2D::ReadMD5FromFile(const FilePath& md5file, MD5::MD5Digest& digest) const
{
    ScopedPtr<File> file(File::Create(md5file, File::OPEN | File::READ));
    if (file)
    {
        auto bytesRead = file->Read(digest.digest.data(), static_cast<uint32>(digest.digest.size()));
        DVASSERT(bytesRead == MD5::MD5Digest::DIGEST_SIZE && "We should always read 16 bytes from md5 file");
        return true;
    }
    else
    {
        return false;
    }
}

bool ResourcePacker2D::WriteMD5ToFile(const FilePath& md5file, const MD5::MD5Digest& digest) const
{
    ScopedPtr<File> file(File::Create(md5file, File::CREATE | File::WRITE));
    DVASSERT(file && "Can't create md5 file");

    auto bytesWritten = file->Write(digest.digest.data(), static_cast<uint32>(digest.digest.size()));
    DVASSERT(bytesWritten == MD5::MD5Digest::DIGEST_SIZE && "16 bytes should be always written for md5 file");

    return true;
}

bool ResourcePacker2D::RecalculateParamsMD5(const String& params, const FilePath& md5file) const
{
    MD5::MD5Digest oldMD5Digest;
    MD5::MD5Digest newMD5Digest;

    bool oldMD5Read = ReadMD5FromFile(md5file, oldMD5Digest);

    MD5::ForData(reinterpret_cast<const uint8*>(params.data()), static_cast<uint32>(params.size()), newMD5Digest);

    WriteMD5ToFile(md5file, newMD5Digest);

    bool isChanged = true;
    if (oldMD5Read)
    {
        isChanged = !(oldMD5Digest == newMD5Digest);
    }
    return isChanged;
}
bool ResourcePacker2D::RecalculateDirMD5(const FilePath& pathname, const FilePath& md5file, bool isRecursive) const
{
    MD5::MD5Digest oldMD5Digest;
    MD5::MD5Digest newMD5Digest;

    bool oldMD5Read = ReadMD5FromFile(md5file, oldMD5Digest);

    MD5::ForDirectory(pathname, newMD5Digest, isRecursive, false);

    WriteMD5ToFile(md5file, newMD5Digest);

    bool isChanged = true;
    if (oldMD5Read)
    {
        isChanged = !(oldMD5Digest == newMD5Digest);
    }
    return isChanged;
}

bool ResourcePacker2D::RecalculateFileMD5(const FilePath& pathname, const FilePath& md5file) const
{
    FilePath md5FileName = FilePath::CreateWithNewExtension(md5file, ".md5");

    MD5::MD5Digest oldMD5Digest;
    MD5::MD5Digest newMD5Digest;

    bool oldMD5Read = ReadMD5FromFile(md5file, oldMD5Digest);

    MD5::ForFile(pathname, newMD5Digest);

    WriteMD5ToFile(md5file, newMD5Digest);

    bool isChanged = true;
    if (oldMD5Read)
    {
        isChanged = !(oldMD5Digest == newMD5Digest);
    }
    return isChanged;
}

DefinitionFile * ResourcePacker2D::ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName, bool twoSideMargin, uint32 texturesMargin)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());
    
    uint32 maxTextureSize = (CommandLineParser::Instance()->IsFlagSet("--tsize4096")) ? 4096 : TexturePacker::DEFAULT_TEXTURE_SIZE;

    bool withAlpha = CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha");
    bool useLayerNames = CommandLineParser::Instance()->IsFlagSet("--useLayerNames");
    
    FilePath psdNameWithoutExtension(processDirectoryPath + psdName);
    psdNameWithoutExtension.TruncateExtension();
    
    IMagickHelper::CroppedData cropped_data;
    
    IMagickHelper::ConvertToPNGCroppedGeometry( psdPathname.GetAbsolutePathname().c_str(), processDirectoryPath.GetAbsolutePathname().c_str() , &cropped_data, true );
    	
    if ( cropped_data.layers_array_size == 0 )
    {
    	AddError(Format("Number of layers is too low: %s", psdPathname.GetAbsolutePathname().c_str()));
    	return nullptr;
    }
    	
    //Logger::FrameworkDebug("psd file: %s wext: %s", psdPathname.c_str(), psdNameWithoutExtension.c_str());
    	
    int width  = cropped_data.layer_width;
    int height = cropped_data.layer_height;
    	
    DefinitionFile * defFile = new DefinitionFile;
    defFile->filename = psdNameWithoutExtension + ".txt";

    defFile->spriteWidth = width;
    defFile->spriteHeight = height;
    defFile->frameCount = static_cast<int>(cropped_data.layers_array_size) - 1;
    defFile->frameRects = new Rect2i[defFile->frameCount];

    for(int k = 1; k < static_cast<int>(cropped_data.layers_array_size); ++k)
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
        {
            defFile->frameRects[k - 1] = Rect2i(cropped_data.layers_array[k].x, cropped_data.layers_array[k].y, width, height);
        }
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

    ScopedPtr<File> file(File::Create(flagsPathname, File::READ | File::OPEN));
    if (!file)
    {
    	AddError(Format("Failed to open file: %s", flagsPathname.GetAbsolutePathname().c_str()));
        return tokens;
    }

    String tokenString = file->ReadLine();
    Split(tokenString, " ", tokens, false);

    return tokens;
}

void ResourcePacker2D::RecursiveTreeWalk(const FilePath & inputPath, const FilePath & outputPath, const Vector<String> & passedFlags)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());

    if (!running)
    {
        return;
    }

    uint64 packTime = SystemTimer::Instance()->AbsoluteMS();

    String inputRelativePath = inputPath.GetRelativePathname(rootDirectory);
    FilePath processDir = rootDirectory + GetProcessFolderName() + inputRelativePath;
    FileSystem::Instance()->CreateDirectory(processDir, true);

    if (forceRepack)
    {
        FileSystem::Instance()->DeleteDirectoryFiles(processDir, false);
    }

    FileSystem::Instance()->CreateDirectory(outputPath);

    Vector<String> currentFlags;

    const auto flagsPathname = inputPath + "flags.txt";
    if (flagsPathname.Exists())
    {
        currentFlags = FetchFlags(flagsPathname);
    }
    else
    {
        currentFlags = passedFlags;
    }

    CommandLineParser::Instance()->SetFlags(currentFlags);

    ScopedPtr<FileList> fileList(new FileList(inputPath));
    fileList->Sort();

    bool inputDirHasFiles = false;
    for (int fi = 0; fi < fileList->GetCount(); ++fi)
    {
        if (!fileList->IsDirectory(fi))
        {
            inputDirHasFiles = true;
            break;
        }
    }

    String mergedFlags;
    Merge(currentFlags, ' ', mergedFlags);
    Logger::FrameworkDebug("Flags applied for current folder: %s", mergedFlags.c_str());

    String mergedParams = mergedFlags;
    mergedParams += String("GPU = ") + GPUFamilyDescriptor::GetGPUName(requestedGPUFamily);
    mergedParams += String("PackerVersion = ") + VERSION;

    bool inputDirModified = RecalculateDirMD5(inputPath, processDir + "dir.md5", false);
    bool paramsModified = RecalculateParamsMD5(mergedParams, processDir + "params.md5");

    bool modified = outputDirModified || inputDirModified || paramsModified;
    if (modified)
    {
        if (inputDirHasFiles)
        {
            AssetCache::CacheItemKey cacheKey;
            if (IsUsingCache())
            {
                ScopedPtr<File> md5File(File::Create(processDir + "dir.md5", File::OPEN | File::READ));
                DVASSERT(md5File);
                auto read = md5File->Read(cacheKey.data(), MD5::MD5Digest::DIGEST_SIZE);
                DVASSERT(read == MD5::MD5Digest::DIGEST_SIZE);

                md5File = File::Create(processDir + "params.md5", File::OPEN | File::READ);
                DVASSERT(md5File);
                read = md5File->Read(cacheKey.data() + MD5::MD5Digest::DIGEST_SIZE, MD5::MD5Digest::DIGEST_SIZE);
                DVASSERT(read == MD5::MD5Digest::DIGEST_SIZE);
            }

            bool needRepack = (false == GetFilesFromCache(cacheKey, inputPath, outputPath));
            if (needRepack)
            {
                List<DefinitionFile*> definitionFileList;

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

                if (clearOutputDirectory)
                {
                    FileSystem::Instance()->DeleteDirectoryFiles(outputPath, false);
                }

                for (int fi = 0; fi < fileList->GetCount() && running; ++fi)
                {
                    if (!fileList->IsDirectory(fi))
                    {
                        FilePath fullname = fileList->GetPathname(fi);
                        if (fullname.IsEqualToExtension(".psd"))
                        {
                            //TODO: check if we need filename or pathname
                            DefinitionFile* defFile = ProcessPSD(processDir, fullname, fullname.GetFilename(), useTwoSideMargin, marginInPixels);
                            if (!defFile)
                            {
                                // An error occured while converting this PSD file - cancel converting in this directory.
                                break;
                            }

                            definitionFileList.push_back(defFile);
                        }
                        else if (isLightmapsPacking && fullname.IsEqualToExtension(".png"))
                        {
                            DefinitionFile* defFile = new DefinitionFile();
                            defFile->LoadPNG(fullname, processDir);
                            definitionFileList.push_back(defFile);
                        }
                        else if (fullname.IsEqualToExtension(".pngdef"))
                        {
                            DefinitionFile* defFile = new DefinitionFile();
                            if (defFile->LoadPNGDef(fullname, processDir, useTwoSideMargin, marginInPixels))
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

                if (!definitionFileList.empty())
                {
                    TexturePacker packer;
                    packer.SetConvertQuality(quality);

                    if (isLightmapsPacking)
                    {
                        packer.UseOnlySquareTextures();
                        packer.SetMaxTextureSize(2048);
                    }
                    else
                    {
                        if (CommandLineParser::Instance()->IsFlagSet("--square"))
                        {
                            packer.UseOnlySquareTextures();
                        }
                        if (CommandLineParser::Instance()->IsFlagSet("--tsize4096"))
                        {
                            packer.SetMaxTextureSize(4096);
                        }
                    }

                    packer.SetTwoSideMargin(useTwoSideMargin);
                    packer.SetTexturesMargin(marginInPixels);

                    if (CommandLineParser::Instance()->IsFlagSet("--split"))
                    {
                        packer.PackToTexturesSeparate(outputPath, definitionFileList, requestedGPUFamily);
                    }
                    else
                    {
                        packer.PackToTextures(outputPath, definitionFileList, requestedGPUFamily);
                    }

                    Set<String> currentErrors = packer.GetErrors();
                    if (!currentErrors.empty())
                    {
                        errors.insert(currentErrors.begin(), currentErrors.end());
                    }
                }

                packTime = SystemTimer::Instance()->AbsoluteMS() - packTime;

                if (Core::Instance()->IsConsoleMode())
                {
                    Logger::Info("[%d files packed with flags: %s]", (int)definitionFileList.size(), mergedFlags.c_str());
                }

                const char* result = definitionFileList.empty() ? "[unchanged]" : "[REPACKED]";
                Logger::Info("[%s - %.2lf secs] - %s", inputPath.GetAbsolutePathname().c_str(), (float64)packTime / 1000.0f, result);

                for_each(definitionFileList.begin(), definitionFileList.end(), SafeDelete<DefinitionFile>);

                AddFilesToCache(cacheKey, inputPath, outputPath);
            }
        }
        else if (outputDirModified || inputDirModified)
        {
            Logger::Info("[%s] - empty directory. Clearing output folder", inputPath.GetAbsolutePathname().c_str());
            FileSystem::Instance()->DeleteDirectoryFiles(outputPath, false);
        }
    }
    else
    {
        Logger::Info("[%s] - unchanged", inputPath.GetAbsolutePathname().c_str());
    }

    if (false == CommandLineParser::Instance()->IsFlagSet("--recursive"))
    {
        currentFlags.clear();
    }

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

                    RecursiveTreeWalk(input, output, currentFlags);
                }
    		}
    	}
    }
}

bool ResourcePacker2D::GetFilesFromCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath)
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache client in win uap
    return false;
#else
    if (!IsUsingCache())
    {
        return false;
    }

    auto oldDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    FileSystem::Instance()->SetCurrentWorkingDirectory(cacheClientTool.GetDirectory());
    SCOPE_EXIT
    {
        FileSystem::Instance()->SetCurrentWorkingDirectory(oldDir);
    };

    Vector<String> arguments;
    arguments.push_back("get");

    arguments.push_back("-h");
    arguments.push_back(AssetCache::KeyToString(key));

    arguments.push_back("-f");
    arguments.push_back(outputPath.GetAbsolutePathname());

    if (!cacheClientIp.empty())
    {
        arguments.push_back("-ip");
        arguments.push_back(cacheClientIp);
    }

    if (!cacheClientPort.empty())
    {
        arguments.push_back("-p");
        arguments.push_back(cacheClientPort);
    }

    if (!cacheClientTimeout.empty())
    {
        arguments.push_back("-t");
        arguments.push_back(cacheClientTimeout);
    }

    uint64 getTime = SystemTimer::Instance()->AbsoluteMS();
    Process cacheClient(cacheClientTool, arguments);
    if (cacheClient.Run(false))
    {
        cacheClient.Wait();

        auto exitCode = cacheClient.GetExitCode();
        getTime = SystemTimer::Instance()->AbsoluteMS() - getTime;

        if (exitCode == AssetClientCode::OK)
        {
            Logger::Info("[%s - %.2lf secs] - GOT FROM CACHE", inputPath.GetAbsolutePathname().c_str(), (float64)(getTime) / 1000.0f);
            return true;
        }
        else
        {
            Logger::Info("[%s - %.2lf secs] - attempted to retrieve from cache, result code %d (%s)",
                         inputPath.GetAbsolutePathname().c_str(),
                         (float64)(getTime) / 1000.0f,
                         exitCode,
                         GetCodeAsString(static_cast<AssetClientCode>(exitCode)).c_str());
            const String& procOutput = cacheClient.GetOutput();
            if (!procOutput.empty())
            {
                Logger::FrameworkDebug("\nCacheClientLog: %s", procOutput.c_str());
            }

            if (exitCode == AssetClientCode::TIMEOUT)
            {
                isUsingCache = false;
            }

            return false;
        }
    }
    else
    {
        Logger::Warning("Can't run process '%s'", cacheClientTool.GetAbsolutePathname().c_str());
        return false;
    }
#endif
}

bool ResourcePacker2D::AddFilesToCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath)
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache client in win uap
    return false;
#else
    if (!IsUsingCache())
    {
        return false;
    }

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
            if (fileListString.empty() == false)
            {
                fileListString += String(",");
            }

            fileListString += outFilesList->GetPathname(fi).GetAbsolutePathname();
        }
    }

    if (fileListString.empty() == false)
    {
        Vector<String> arguments;
        arguments.push_back("add");

        arguments.push_back("-h");
        arguments.push_back(AssetCache::KeyToString(key));

        arguments.push_back("-f");
        arguments.push_back(fileListString);

        if (!cacheClientIp.empty())
        {
            arguments.push_back("-ip");
            arguments.push_back(cacheClientIp);
        }

        if (!cacheClientPort.empty())
        {
            arguments.push_back("-p");
            arguments.push_back(cacheClientPort);
        }

        if (!cacheClientTimeout.empty())
        {
            arguments.push_back("-t");
            arguments.push_back(cacheClientTimeout);
        }
        else if (outFilesList->GetFileCount() > 20)
        {
            arguments.push_back("-t");
            arguments.push_back("5"); //enlarge default timeout
        }

        uint64 getTime = SystemTimer::Instance()->AbsoluteMS();
        Process cacheClient(cacheClientTool, arguments);
        if (cacheClient.Run(false))
        {
            cacheClient.Wait();

            auto exitCode = cacheClient.GetExitCode();
            getTime = SystemTimer::Instance()->AbsoluteMS() - getTime;

            if (exitCode == AssetClientCode::OK)
            {
                Logger::Info("[%s - %.2lf secs] - ADDED TO CACHE", inputPath.GetAbsolutePathname().c_str(), (float64)(getTime) / 1000.0f);
                return true;
            }
            else
            {
                Logger::Info("[%s - %.2lf secs] - attempted to add to cache, result code %d (%s)", inputPath.GetAbsolutePathname().c_str(),
                             (float64)(getTime) / 1000.0f,
                             exitCode,
                             GetCodeAsString(static_cast<AssetClientCode>(exitCode)).c_str());
                const String& procOutput = cacheClient.GetOutput();
                if (!procOutput.empty())
                {
                    Logger::FrameworkDebug("\nCacheClientLog: %s", procOutput.c_str());
                }

                if (exitCode == AssetClientCode::TIMEOUT)
                {
                    isUsingCache = false;
                }

                return false;
            }
        }
        else
        {
            Logger::Warning("Can't run process '%s'", cacheClientTool.GetAbsolutePathname().c_str());
            return false;
        }
    }
    else
    {
        Logger::FrameworkDebug("Dir [%s] is empty. Nothing to add to cache", outputPath.GetAbsolutePathname().c_str());
        return false;
    }
#endif
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

void ResourcePacker2D::SetCacheClientTool(const DAVA::FilePath& path, const String& ip, const String& port, const String& timeout)
{
    cacheClientTool = path;
    cacheClientIp = ip;
    cacheClientPort = port;
    cacheClientTimeout = timeout;
    isUsingCache = !cacheClientTool.IsEmpty();
}

void ResourcePacker2D::ClearCacheClientTool()
{
    cacheClientTool = "";
    cacheClientIp.clear();
    cacheClientPort.clear();
    cacheClientTimeout.clear();
    isUsingCache = false;
}
};
