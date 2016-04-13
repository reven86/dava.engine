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
#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"
#include "Platform/SystemTimer.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"
#include "Render/GPUFamilyDescriptor.h"
#include "AssetCache/AssetCache.h"
#include "AssetCache/AssetCacheConstants.h"
#include "Platform/Process.h"

#include "AssetCache/AssetCacheClient.h"

namespace DAVA
{
const String ResourcePacker2D::VERSION = "0.0.3";
const String ResourcePacker2D::INTERNAL_LIBPSD_VERSION = "0.0.1";

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
void ResourcePacker2D::InitFolders(const FilePath& inputPath, const FilePath& outputPath)
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

    String alg = CommandLineParser::Instance()->GetCommandParam("-alg");
    if (alg.empty() || CompareCaseInsensitive(alg, "maxrect") == 0)
    {
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_AREA_FIT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_LONG_SIDE_FIT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_SHORT_SIDE_FIT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BOTTOM_LEFT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRRECT_BEST_CONTACT_POINT);
    }
    else if (CompareCaseInsensitive(alg, "maxrect_fast") == 0)
    {
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_AREA_FIT);
    }
    else if (CompareCaseInsensitive(alg, "basic") == 0)
    {
        packAlgorithms.push_back(PackingAlgorithm::ALG_BASIC);
    }
    else
    {
        AddError(Format("Unknown algorithm: '%s'", alg.c_str()));
        return;
    }

    requestedGPUFamily = forGPU;
    outputDirModified = false;

    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
    std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);

    FilePath processDirectoryPath = rootDirectory + GetProcessFolderName();
    FileSystem::Instance()->CreateDirectory(processDirectoryPath, true);

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

void ResourcePacker2D::WriteMD5ToFile(const FilePath& md5file, const MD5::MD5Digest& digest) const
{
    ScopedPtr<File> file(File::Create(md5file, File::CREATE | File::WRITE));
    DVASSERT(file && "Can't create md5 file");

    auto bytesWritten = file->Write(digest.digest.data(), static_cast<uint32>(digest.digest.size()));
    DVASSERT(bytesWritten == MD5::MD5Digest::DIGEST_SIZE && "16 bytes should be always written for md5 file");
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

Vector<String> ResourcePacker2D::FetchFlags(const FilePath& flagsPathname)
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

void ResourcePacker2D::RecursiveTreeWalk(const FilePath& inputPath, const FilePath& outputPath, const Vector<String>& passedFlags)
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
    if (FileSystem::Instance()->Exists(flagsPathname))
    {
        currentFlags = FetchFlags(flagsPathname);
    }
    else
    {
        currentFlags = passedFlags;
    }

    CommandLineParser::Instance()->SetFlags(currentFlags);
    String mergedFlags;
    Merge(currentFlags, ' ', mergedFlags);

    String packingParams = mergedFlags;
    packingParams += String("GPU = ") + GPUFamilyDescriptor::GetGPUName(requestedGPUFamily);
    packingParams += String("PackerVersion = ") + VERSION;
    packingParams += String("LibPSDVersion = ") + INTERNAL_LIBPSD_VERSION;
    for (const auto& algorithm : packAlgorithms)
    {
        packingParams += String("PackerAlgorithm = ") + GlobalEnumMap<DAVA::PackingAlgorithm>::Instance()->ToString(static_cast<int>(algorithm));
    }

    ScopedPtr<FileList> fileList(new FileList(inputPath));
    fileList->Sort();

    bool inputDirHasFiles = false;
    uint64 allFilesSize = 0;
    uint32 allFilesCount = 0;

    static const Vector<String> ignoredFileNames = { ".DS_Store", "flags.txt", "Thumbs.db", ".gitignore" };
    auto IsFileIgnoredByName = [](const Vector<String>& ignoredFileNames, const String& filename)
    {
        auto found = std::find_if(ignoredFileNames.begin(), ignoredFileNames.end(), [&filename](const String& name)
                                  {
                                      return (CompareCaseInsensitive(filename, name) == 0);
                                  });

        return (found != ignoredFileNames.end());
    };

    for (int fi = 0; fi < fileList->GetCount(); ++fi)
    {
        if (!fileList->IsDirectory(fi))
        {
            String fileName = fileList->GetFilename(fi);
            if (IsFileIgnoredByName(ignoredFileNames, fileName))
            {
                continue;
            }

            inputDirHasFiles = true;

            packingParams += fileName;
            allFilesSize += fileList->GetFileSize(fi);
            ++allFilesCount;
        }
    }

    packingParams += Format("FilesSize = %llu", allFilesSize);
    packingParams += Format("FilesCount = %u", allFilesCount);

    bool inputDirModified = RecalculateDirMD5(inputPath, processDir + "dir.md5", false);
    bool paramsModified = RecalculateParamsMD5(packingParams, processDir + "params.md5");

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
                // read textures margins settings
                bool useTwoSideMargin = CommandLineParser::Instance()->IsFlagSet("--add2sidepixel");
                uint32 marginInPixels = useTwoSideMargin ? 0 : 1;
                if (CommandLineParser::Instance()->IsFlagSet("--add0pixel"))
                    marginInPixels = 0;
                else if (CommandLineParser::Instance()->IsFlagSet("--add1pixel"))
                    marginInPixels = 1;
                else if (CommandLineParser::Instance()->IsFlagSet("--add2pixel"))
                    marginInPixels = 2;
                else if (CommandLineParser::Instance()->IsFlagSet("--add4pixel"))
                    marginInPixels = 4;

                uint32 maxTextureSize = (CommandLineParser::Instance()->IsFlagSet("--tsize4096")) ? 4096 : TexturePacker::DEFAULT_TEXTURE_SIZE;
                bool withAlpha = CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha");
                bool useLayerNames = CommandLineParser::Instance()->IsFlagSet("--useLayerNames");

                if (clearOutputDirectory)
                {
                    FileSystem::Instance()->DeleteDirectoryFiles(outputPath, false);
                }

                DefinitionFile::Collection definitionFileList;
                definitionFileList.reserve(fileList->GetCount());
                for (int32 fi = 0; fi < fileList->GetCount() && running; ++fi)
                {
                    if (fileList->IsDirectory(fi))
                        continue;

                    definitionFileList.emplace_back(new DefinitionFile());

                    DefinitionFile::Pointer& defFile = definitionFileList.back();
                    bool shouldAcceptFile = false;

                    FilePath fullname = fileList->GetPathname(fi);
                    if (fullname.IsEqualToExtension(".psd"))
                    {
                        shouldAcceptFile = defFile->LoadPSD(fullname, processDir, maxTextureSize, withAlpha, useLayerNames);
                    }
                    else if (isLightmapsPacking && fullname.IsEqualToExtension(".png"))
                    {
                        shouldAcceptFile = true;
                        defFile->LoadPNG(fullname, processDir);
                    }
                    else if (fullname.IsEqualToExtension(".pngdef"))
                    {
                        shouldAcceptFile = defFile->LoadPNGDef(fullname, processDir);
                    }

                    if (shouldAcceptFile == false)
                    {
                        definitionFileList.pop_back();
                    }
                }

                if (!definitionFileList.empty())
                {
                    TexturePacker packer;
                    packer.SetConvertQuality(quality);

                    if (isLightmapsPacking)
                    {
                        packer.SetUseOnlySquareTextures();
                        packer.SetMaxTextureSize(2048);
                    }
                    else
                    {
                        if (CommandLineParser::Instance()->IsFlagSet("--square"))
                        {
                            packer.SetUseOnlySquareTextures();
                        }
                        if (CommandLineParser::Instance()->IsFlagSet("--tsize4096"))
                        {
                            packer.SetMaxTextureSize(4096);
                        }
                    }

                    packer.SetTwoSideMargin(useTwoSideMargin);
                    packer.SetTexturesMargin(marginInPixels);
                    packer.SetAlgorithms(packAlgorithms);

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
                    Logger::Info("[%u files packed with flags: %s]", static_cast<uint32>(definitionFileList.size()), mergedFlags.c_str());
                }

                const char* result = definitionFileList.empty() ? "[unchanged]" : "[REPACKED]";
                Logger::Info("[%s - %.2lf secs] - %s", inputPath.GetAbsolutePathname().c_str(),
                             static_cast<float64>(packTime) / 1000.0, result);

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

    const auto& flagsToPass = CommandLineParser::Instance()->IsFlagSet("--recursive") ? currentFlags : passedFlags;

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

                    RecursiveTreeWalk(input, output, flagsToPass);
                }
            }
        }
    }
}

void ResourcePacker2D::SetCacheClient(AssetCacheClient* cacheClient_, const String& comment)
{
    cacheClient = cacheClient_;

    cacheItemDescription.machineName = WStringToString(DeviceInfo::GetName());

    DateTime timeNow = DateTime::Now();
    cacheItemDescription.creationDate = WStringToString(timeNow.GetLocalizedDate()) + "_" + WStringToString(timeNow.GetLocalizedTime());

    cacheItemDescription.comment = comment;
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

    AssetCache::Error requested = cacheClient->RequestFromCacheSynchronously(key, outputPath);
    if (requested == AssetCache::Error::NO_ERRORS)
    {
        return true;
    }
    else
    {
        Logger::Info("%s - failed to retrieve from cache(%s)", inputPath.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(requested).c_str());
    }

    return false;
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

    AssetCache::CachedItemValue value;

    ScopedPtr<FileList> outFilesList(new FileList(outputPath));
    for (int fi = 0; fi < outFilesList->GetCount(); ++fi)
    {
        if (!outFilesList->IsDirectory(fi))
        {
            value.Add(outFilesList->GetPathname(fi));
        }
    }

    if (!value.IsEmpty())
    {
        value.UpdateValidationData();
        value.SetDescription(cacheItemDescription);

        AssetCache::Error added = cacheClient->AddToCacheSynchronously(key, value);
        if (added == AssetCache::Error::NO_ERRORS)
        {
            Logger::Info("%s - added to cache", inputPath.GetAbsolutePathname().c_str());
            return true;
        }
        else
        {
            Logger::Info("%s - failed to add to cache (%s)", inputPath.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(added).c_str());
        }
    }
    else
    {
        Logger::Info("%s - empty folder", inputPath.GetAbsolutePathname().c_str());
    }

    return false;
        
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

bool ResourcePacker2D::IsUsingCache() const
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache in win uap
    return false;
#else
    return (cacheClient != nullptr) && cacheClient->IsConnected();
#endif
}
};
