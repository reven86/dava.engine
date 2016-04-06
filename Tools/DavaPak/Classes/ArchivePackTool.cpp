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


#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Utils/Utils.h"
#include "Utils/MD5.h"
#include "Functional/Function.h"
#include "Archiver/Archiver.h"

#include "ArchivePackTool.h"
#include "Utils.h"

using namespace DAVA;

namespace ArchivePackToolDetail
{
void OnOneFilePacked(const ResourceArchive::FileInfo& info)
{
    static String deviceName = WStringToString(DeviceInfo::GetName());
    DateTime dateTime;
    String date = WStringToString(dateTime.GetLocalizedDate());
    String time = WStringToString(dateTime.GetLocalizedTime());
    Logger::Info("%s | %s %s | Packed %s, orig size %d, compressed size %d, compression: %s",
                 deviceName, date, time,
                 info.relativeFilePath, info.originalSize, info.compressedSize,
                 ResourceArchiveToolUtils::ToString(info.compressionType).c_str());
}

} // namespace ArchivePackToolDetail

// void ArchivePackTool::OnOneFilePacked(const ResourceArchive::FileInfo& info)
// {
//     Logger::Info("Packing %s, compressed size %d, orig size %d, type %s",
//         info.fileName, info.compressedSize, info.originalSize,
//         ResourceArchiveToolUtils::ToString(info.compressionType).c_str());
// }

void ArchivePackTool::CollectAllFilesInDirectory(const String& pathDirName, Vector<String>& collectedFiles)
{
    ScopedPtr<FileList> fileList(new FileList((pathDirName.empty() ? "." : pathDirName), addHidden));
    for (auto file = 0; file < fileList->GetCount(); ++file)
    {
        if (fileList->IsDirectory(file) && !fileList->IsNavigationDirectory(file))
        {
            String directoryName = fileList->GetFilename(file);
            String subDir = pathDirName + directoryName + '/';
            CollectAllFilesInDirectory(subDir, collectedFiles);
        }
        else
        {
            if (fileList->IsHidden(file) && addHidden == false)
            {
                continue;
            }
            String pathname = pathDirName + fileList->GetFilename(file);
            collectedFiles.push_back(pathname);
        }
    }
}

static const String DEFAULT_CACHE_IP = "";
static const uint32 DEFAULT_CACHE_PORT = 44234;
static const uint32 DEFAULT_CACHE_TIMEOUT_MS = 5000;

ArchivePackTool::ArchivePackTool()
    : CommandLineTool("-pack")
{
    options.AddOption(OptionNames.Compression, VariantType(String("lz4hc")), "default compression method, lz4hc - default");
    options.AddOption(OptionNames.AddHidden, VariantType(false), "add hidden files to pack list");
    options.AddOption(OptionNames.UseCache, VariantType(false), "use asset cache");
    options.AddOption(OptionNames.Ip, VariantType(DEFAULT_CACHE_IP), "asset cache ip");
    options.AddOption(OptionNames.Port, VariantType(DEFAULT_CACHE_PORT), "asset cache port");
    options.AddOption(OptionNames.Timeout, VariantType(DEFAULT_CACHE_TIMEOUT_MS), "asset cache timeout");
    options.AddOption(OptionNames.LogFile, VariantType(""), "package process log file");
    options.AddOption(OptionNames.Dir, VariantType(String("")), "source files directory");
    options.AddOption(OptionNames.SpecFiles, VariantType(String("")), "text files containing list of source files", true);
    options.AddOption(OptionNames.Files, VariantType(String("")), "source files", true);
    options.AddArgument("packfile");
}

bool ArchivePackTool::ConvertOptionsToParamsInternal()
{
    compressionStr = options.GetOption(OptionNames.Compression).AsString();
    if (!ResourceArchiveToolUtils::ToPackType(compressionStr, compressionType))
    {
        Logger::Error("Invalid compression type: '%s'", compressionStr.c_str());
        return false;
    }

    addHidden = options.GetOption(OptionNames.AddHidden).AsBool();
    useCache = options.GetOption(OptionNames.Ip).AsBool();
    ip = options.GetOption(OptionNames.Ip).AsString();
    port = options.GetOption(OptionNames.Port).AsUInt32();
    timeout = options.GetOption(OptionNames.Timeout).AsUInt32();
    logFileName = options.GetOption(OptionNames.LogFile).AsString();

    source = Source::Unknown;

    srcDir = options.GetOption(OptionNames.Dir).AsString();
    if (!srcDir.IsEmpty())
    {
        srcDir.MakeDirectoryPathname();
        source = Source::UseDir;
    }

    uint32 specCount = options.GetOptionValuesCount(OptionNames.SpecFiles);
    if (specCount > 0)
    {
        if (source != Source::Unknown)
        {
            Logger::Error("Unexpected parameter: %s", OptionNames.SpecFiles.c_str());
            return false;
        }

        source = Source::UseSpecFiles;

        for (uint32 n = 0; n < specCount; ++n)
        {
            specFiles.push_back(options.GetOption(OptionNames.SpecFiles, n).AsString());
        }
    }

    uint32 srcFilesCount = options.GetOptionValuesCount(OptionNames.Files);
    if (specCount > 0)
    {
        if (source != Source::Unknown)
        {
            Logger::Error("Unexpected parameter: %s", OptionNames.Files.c_str());
            return false;
        }

        source = Source::UseSrcFiles;

        for (uint32 n = 0; n < srcFilesCount; ++n)
        {
            srcFiles.push_back(options.GetOption(OptionNames.Files, n).AsString());
        }
    }

    packFileName = options.GetArgument("pakfile");
    if (packFileName.empty())
    {
        Logger::Error("pakfile param is not specified");
        return false;
    }

    return true;
}

void ArchivePackTool::ProcessInternal()
{
    Vector<String> files;

    FilePath initialDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    FilePath packFilePath = initialDir + packFileName;
    FilePath logFilePath = (logFileName.empty() ? FilePath() : initialDir + logFileName);

    switch (source)
    {
    case Source::UseDir:
    {
        Logger::Info("Packing '%s' into %s", srcDir.GetAbsolutePathname().c_str(), packFileName.c_str());

        if (!FileSystem::Instance()->SetCurrentWorkingDirectory(srcDir))
        {
            Logger::Error("Can't change current dir to '%s'", srcDir.GetAbsolutePathname().c_str());
            return;
        }

        CollectAllFilesInDirectory("", files);
        FileSystem::Instance()->SetCurrentWorkingDirectory(initialDir);
        break;
    }
    case Source::UseSpecFiles:
    {
        for (const String& filename : specFiles)
        {
            ScopedPtr<File> specFile(File::Create(filename, File::OPEN | File::READ));
            if (specFile)
            {
                while (!specFile->IsEof())
                {
                    String str;
                    specFile->ReadString(str);
                    files.push_back(str);
                }
            }
            else
            {
                Logger::Error("Can't open specfile %s", filename.c_str());
                return;
            }
        }
        break;
    }
    case Source::UseSrcFiles:
    {
        files.swap(srcFiles);
    }
    default:
    {
        DVASSERT_MSG(false, Format("Incorrect source type: %d", source));
        return;
    }
    }

    if (files.empty())
    {
        Logger::Error("No source files");
        return;
    }

    // std::stable_sort(files.begin(), files.end());

    //     if (FileSystem::Instance()->Exists(packFilePath))
    //     {
    //         Logger::Info("%s is already exist. Deleting it", packFilePath.GetFilename().c_str());
    //         if (FileSystem::Instance()->DeleteFile(packFilePath.GetAbsolutePathname().c_str()) == false)
    //         {
    //             Logger::Error("Can't delete existing pakfile");
    //             return;
    //         }
    //     }

    AssetCache::CacheItemKey key;
    if (useCache)
    {
        ConstructCacheKey(key, files, compressionStr);

        if (RetrieveFromCache(key, packFilePath, logFilePath))
        {
            return;
        }
    }

    if (!logFilePath.IsEmpty())
    {
        Logger::Instance()->SetLogPathname(logFilePath);
    }

    if (Archiver::CreatePack(packFilePath.GetAbsolutePathname(), files, ArchivePackToolDetail::OnOneFilePacked))
    {
        Logger::Info("Packing done");

        if (useCache)
        {
            AddToCache(key, packFilePath, logFilePath);
        }
        return;
    }
    else
    {
        Logger::Info("Packing failed");
        return;
    }
}

MD5::MD5Digest CalculateFilesMD5(const Vector<String>& files)
{
    MD5 md5;
    md5.Init();
    for (const String& filename : files)
    {
        md5.Update(reinterpret_cast<const uint8*>(filename.data()), static_cast<uint32>(filename.size()));

        MD5::MD5Digest fileDigest;
        MD5::ForFile(filename, fileDigest);
        md5.Update(fileDigest.digest.data(), static_cast<uint32>(fileDigest.digest.size()));
    }

    md5.Final();
    return md5.GetDigest();
}

void ArchivePackTool::ConstructCacheKey(AssetCache::CacheItemKey& key, const Vector<String>& files, const DAVA::String& compression) const
{
    MD5::MD5Digest filesMD5 = CalculateFilesMD5(files);
    MD5::MD5Digest paramsMD5;
    MD5::ForData(reinterpret_cast<const uint8*>(compression.data()), compression.size(), paramsMD5);
    AssetCache::SetKeyPart1(key, filesMD5);
    AssetCache::SetKeyPart2(key, paramsMD5);
}

bool ArchivePackTool::RetrieveFromCache(const AssetCache::CacheItemKey& key, const FilePath& pathToPackage, const FilePath& pathToLog) const
{
    // todo: retrieve from cache using Viktor's AssetCache interface
    return false;
}

bool ArchivePackTool::AddToCache(const AssetCache::CacheItemKey& key, const FilePath& pathToPack, const FilePath& pathToLog) const
{
    // todo: add to cache using Viktor's AssetCache interface
    return false;
}
