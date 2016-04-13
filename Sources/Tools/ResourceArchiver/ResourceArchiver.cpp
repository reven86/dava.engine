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


#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileList.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "Base/UniquePtr.h"
#include "Utils/Utils.h"
#include "Compression/LZ4Compressor.h"
#include "Compression/ZipCompressor.h"
#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"

#include "AssetCache/AssetCache.h"
#include "AssetCache/AssetCacheClient.h"

#include "ResourceArchiver/ResourceArchiver.h"

#include <algorithm>

namespace DAVA
{
namespace ResourceArchiver
{
const UnorderedMap<String, Compressor::Type> packTypes =
{ { "lz4", Compressor::Type::Lz4 },
  { "lz4hc", Compressor::Type::Lz4HC },
  { "rfc1951", Compressor::Type::RFC1951 },
  { "none", Compressor::Type::None } };

bool StringToCompressType(const String& compressionStr, Compressor::Type& type)
{
    const auto& found = packTypes.find(compressionStr);
    if (found != packTypes.end())
    {
        type = found->second;
        return true;
    }
    else
    {
        return false;
    }
}

String CompressTypeToString(Compressor::Type packType)
{
    for (const auto& type : packTypes)
    {
        if (type.second == packType)
        {
            return type.first;
        }
    }
    return String();
}

struct CollectedFile
{
    FilePath absPath;
    String archivePath;
};

void CollectAllFilesInDirectory(const FilePath& dirPath, const String& dirArchivePath, bool addHidden, Vector<CollectedFile>& collectedFiles)
{
    bool fileOrDirAdded = false;

    UniquePtr<FileList> fileList(new FileList(dirPath, addHidden));
    for (auto file = 0; file < fileList->GetCount(); ++file)
    {
        if (fileList->IsNavigationDirectory(file))
        {
            continue;
        }

        if (fileList->IsDirectory(file))
        {
            fileOrDirAdded = true;
            String directoryName = fileList->GetFilename(file);
            FilePath subDirAbsolute = dirPath + (directoryName + '/');
            String subDirArchive = dirArchivePath + (directoryName + '/');
            CollectAllFilesInDirectory(subDirAbsolute, subDirArchive, addHidden, collectedFiles);
        }
        else
        {
            if (fileList->IsHidden(file) && addHidden == false)
            {
                continue;
            }
            fileOrDirAdded = true;

            CollectedFile collectedFile;
            collectedFile.absPath = fileList->GetPathname(file);
            collectedFile.archivePath = dirArchivePath + fileList->GetFilename(file);
            collectedFiles.push_back(collectedFile);
        }
    }

    if (fileOrDirAdded == false) // add empty folder to preserve file tree hierarchy as-is
    {
        CollectedFile collectedFile;
        collectedFile.absPath = FilePath();
        collectedFile.archivePath = dirArchivePath;
        collectedFiles.push_back(collectedFile);
    }
}

bool WriteHeaderBlock(File* outputFile, const PackFormat::PackFile::HeaderBlock& headerBlock)
{
    DVASSERT(outputFile != nullptr);

    uint32 sizeToWrite = sizeof(PackFormat::PackFile::HeaderBlock);
    uint32 written = outputFile->Write(&headerBlock, sizeToWrite);
    if (written != sizeToWrite)
    {
        LOG_ERROR("Can't write header block to archive");
        return false;
    }

    return true;
}

bool WriteNamesBlock(File* outputFile, const PackFormat::PackFile::NamesBlock& namesBlock)
{
    DVASSERT(outputFile != nullptr);

    uint32 sizeToWrite = static_cast<uint32>(namesBlock.sortedNamesLz4hc.size());
    uint32 written = outputFile->Write(namesBlock.sortedNamesLz4hc.data(), sizeToWrite);
    if (written != sizeToWrite)
    {
        LOG_ERROR("Can't write filenames block to archive");
        return false;
    }

    return true;
}

bool WriteFilesDataBlock(File* outputFile, const PackFormat::PackFile::FilesDataBlock& filesDataBlock)
{
    DVASSERT(outputFile != nullptr);

    uint32 sizeToWrite = static_cast<uint32>(filesDataBlock.files.size() * sizeof(filesDataBlock.files[0]));
    uint32 written = outputFile->Write(filesDataBlock.files.data(), sizeToWrite);
    if (written != sizeToWrite)
    {
        LOG_ERROR("Can't write file table block to archive");
        return false;
    }

    return true;
}

bool WriteRawData(File* outputFile, const Vector<uint8>& srcBuffer)
{
    DVASSERT(outputFile != nullptr);

    uint32 sizeToWrite = static_cast<uint32>(srcBuffer.size());
    uint32 written = outputFile->Write(srcBuffer.data(), sizeToWrite);
    if (written != sizeToWrite)
    {
        LOG_ERROR("Can't write data block to archive");
        return false;
    }

    return true;
}

Vector<uint8> SpliceFileNames(const Vector<CollectedFile>& collectedFiles)
{
    Vector<uint8> sortedNamesOriginal;
    StringStream ss;
    for (const CollectedFile& file : collectedFiles)
    {
        ss << file.archivePath << '\0';
    }
    String strNames(ss.str());
    sortedNamesOriginal.assign(strNames.begin(), strNames.end());
    return sortedNamesOriginal;
}

MD5::MD5Digest CalculateSourcesMD5(const Vector<CollectedFile>& collectedFiles)
{
    MD5 md5;
    md5.Init();
    for (const CollectedFile& collectedFile : collectedFiles)
    {
        md5.Update(reinterpret_cast<const uint8*>(collectedFile.archivePath.data()), static_cast<uint32>(collectedFile.archivePath.size()));

        if (!collectedFile.absPath.IsEmpty())
        {
            MD5::MD5Digest fileDigest;
            MD5::ForFile(collectedFile.absPath, fileDigest);
            md5.Update(fileDigest.digest.data(), static_cast<uint32>(fileDigest.digest.size()));
        }
    }

    md5.Final();
    return md5.GetDigest();
}

MD5::MD5Digest CalculateParamsMD5(Vector<String> params)
{
    MD5 md5;
    md5.Init();
    for (const String& param : params)
    {
        md5.Update(reinterpret_cast<const uint8*>(param.data()), static_cast<uint32>(param.size()));
    }

    md5.Final();
    return md5.GetDigest();
}

void ConstructCacheKeys(AssetCache::CacheItemKey& keyForArchive, AssetCache::CacheItemKey& keyForLog, Vector<CollectedFile>& collectedFiles, const DAVA::String& compression)
{
    MD5::MD5Digest sourcesMD5 = CalculateSourcesMD5(collectedFiles);
    AssetCache::SetPrimaryKey(keyForArchive, sourcesMD5);
    AssetCache::SetPrimaryKey(keyForLog, sourcesMD5);

    MD5::MD5Digest paramsMD5Archive = CalculateParamsMD5({ compression, "key for archive file" });
    MD5::MD5Digest paramsMD5Log = CalculateParamsMD5({ compression, "this one is for log file" });
    AssetCache::SetSecondaryKey(keyForArchive, paramsMD5Archive);
    AssetCache::SetSecondaryKey(keyForArchive, paramsMD5Log);
}

bool RetrieveFileFromCache(AssetCacheClient* assetCacheClient, const AssetCache::CacheItemKey& key, const FilePath& outputFile)
{
    DVASSERT(assetCacheClient != nullptr);

    AssetCache::CachedItemValue retrievedData;
    AssetCache::Error result = assetCacheClient->RequestFromCacheSynchronously(key, &retrievedData);
    if (result == AssetCache::Error::NO_ERRORS)
    {
        if (retrievedData.ExportToFile(outputFile))
        {
            Logger::Info("%s is retrieved from cache", outputFile.GetAbsolutePathname().c_str());
            return true;
        }
        else
        {
            LOG_ERROR("Can't export retrieved data to file %s", outputFile.GetAbsolutePathname().c_str());
            return false;
        }
    }
    else
    {
        Logger::Info("Failed to retrieve %s from cache: %s", outputFile.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(result).c_str());
        return false;
    }
}

bool RetrieveFromCache(AssetCacheClient* assetCacheClient, const AssetCache::CacheItemKey& keyForArchive, const AssetCache::CacheItemKey& keyForLog, const FilePath& pathToArchive, const FilePath& pathToLog)
{
    bool archiveIsRetrieved = RetrieveFileFromCache(assetCacheClient, keyForArchive, pathToArchive);
    if (archiveIsRetrieved && !pathToLog.IsEmpty())
    {
        RetrieveFileFromCache(assetCacheClient, keyForLog, pathToLog);
    }

    return archiveIsRetrieved;
}

bool AddFileToCache(AssetCacheClient* assetCacheClient, const AssetCache::CachedItemValue::Description& description, const AssetCache::CacheItemKey& key, const FilePath& pathToFile)
{
    DVASSERT(assetCacheClient != nullptr);

    AssetCache::CachedItemValue value;
    value.Add(pathToFile);
    value.UpdateValidationData();
    value.SetDescription(description);
    AssetCache::Error result = assetCacheClient->AddToCacheSynchronously(key, value);
    if (result == AssetCache::Error::NO_ERRORS)
    {
        Logger::Info("%s is added to cache", pathToFile.GetAbsolutePathname().c_str());
        return true;
    }
    else
    {
        Logger::Info("Failed to add %s to cache: %s", pathToFile.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(result).c_str());
        return false;
    }
}

bool AddToCache(AssetCacheClient* assetCacheClient, const AssetCache::CacheItemKey& keyForArchive, const AssetCache::CacheItemKey& keyForLog, const FilePath& pathToArchive, const FilePath& pathToLog)
{
    DateTime timeNow = DateTime::Now();
    AssetCache::CachedItemValue::Description cacheItemDescription;
    cacheItemDescription.machineName = WStringToString(DeviceInfo::GetName());
    cacheItemDescription.creationDate = WStringToString(timeNow.GetLocalizedDate()) + "_" + WStringToString(timeNow.GetLocalizedTime());
    cacheItemDescription.comment = Format("Resource archive %s", pathToArchive.GetAbsolutePathname().c_str());

    bool archiveIsAdded = AddFileToCache(assetCacheClient, cacheItemDescription, keyForArchive, pathToArchive);
    if (archiveIsAdded && !pathToLog.IsEmpty())
    {
        AddFileToCache(assetCacheClient, cacheItemDescription, keyForLog, pathToLog);
    }

    return archiveIsAdded;
}

Vector<CollectedFile> CollectFiles(const Vector<String>& sources, bool addHiddenFiles)
{
    Vector<CollectedFile> collectedFiles;

    for (String source : sources)
    {
        source = FilePath::NormalizePathname(source);
        FilePath sourcePath(source);
        bool isAbsolutePath = FilePath::IsAbsolutePathname(source);

        if (sourcePath.IsDirectoryPathname())
        {
            String archivePath = (isAbsolutePath ? (sourcePath.GetLastDirectoryName() + '/') : source);
            CollectAllFilesInDirectory(sourcePath, archivePath, addHiddenFiles, collectedFiles);
        }
        else
        {
            CollectedFile collectedFile;
            collectedFile.absPath = sourcePath;
            collectedFile.archivePath = (isAbsolutePath ? sourcePath.GetBasename() : source);
            collectedFiles.push_back(collectedFile);
        }
    }

    std::stable_sort(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                     {
                         return CompareCaseInsensitive(left.archivePath, right.archivePath) < 0;
                     });

    return collectedFiles;
}

bool Pack(const Vector<CollectedFile>& collectedFiles, DAVA::Compressor::Type compressionType, const FilePath& archivePath)
{
    PackFormat::PackFile packFile;
    PackFormat::PackFile::HeaderBlock& headerBlock = packFile.header;
    PackFormat::PackFile::NamesBlock& namesBlock = packFile.names;
    PackFormat::PackFile::FilesDataBlock& filesDataBlock = packFile.filesData;

    auto& fileTable = filesDataBlock.files;
    fileTable.resize(collectedFiles.size());

    headerBlock.marker = PackFormat::FileMarker;
    headerBlock.numFiles = static_cast<uint32>(fileTable.size());

    Vector<uint8> sortedNamesOriginal = SpliceFileNames(collectedFiles);
    namesBlock.sortedNamesLz4hc.reserve(sortedNamesOriginal.size());

    if (!Compressor::GetCompressor(Compressor::Type::Lz4HC)->Compress(sortedNamesOriginal, namesBlock.sortedNamesLz4hc))
    {
        LOG_ERROR("Can't compress names block");
        return false;
    }

    headerBlock.namesBlockSizeCompressedLZ4HC = static_cast<uint32>(namesBlock.sortedNamesLz4hc.size());
    headerBlock.namesBlockSizeOriginal = static_cast<uint32>(sortedNamesOriginal.size());

    headerBlock.startNamesBlockPosition = sizeof(PackFormat::PackFile::HeaderBlock);
    headerBlock.startFilesDataBlockPosition = headerBlock.startNamesBlockPosition + headerBlock.namesBlockSizeCompressedLZ4HC;

    headerBlock.filesTableBlockSize = static_cast<uint32>(fileTable.size() * sizeof(PackFormat::PackFile::FilesDataBlock::Data));
    headerBlock.startPackedFilesBlockPosition = headerBlock.startFilesDataBlockPosition + headerBlock.filesTableBlockSize;

    UniquePtr<File> outputFile(File::Create(archivePath, File::CREATE | File::WRITE));
    if (!outputFile)
    {
        LOG_ERROR("Can't create %s", archivePath.GetAbsolutePathname().c_str());
        return false;
    }

    if (!WriteHeaderBlock(outputFile, headerBlock) || !WriteNamesBlock(outputFile, namesBlock) || !WriteFilesDataBlock(outputFile, filesDataBlock))
    {
        return false;
    }

    uint32 dataOffset = headerBlock.startPackedFilesBlockPosition;

    Vector<uint8> origFileBuffer;
    Vector<uint8> compressedFileBuffer;

    std::unique_ptr<Compressor> compressor;
    if (compressionType != Compressor::Type::None)
    {
        compressor = Compressor::GetCompressor(compressionType);
        DVASSERT_MSG(compressor, Format("Can't get '%s' compressor", CompressTypeToString(compressionType).c_str()));
    }

    for (size_t i = 0, filesSize = collectedFiles.size(); i < filesSize; ++i)
    {
        const CollectedFile& collectedFile = collectedFiles[i];
        PackFormat::FileTableEntry& fileEntry = fileTable[i];
        fileEntry = { 0 };

        if (collectedFile.absPath.IsEmpty()) // it's an empty folder, nothing to compress
        {
            fileEntry.packType = Compressor::Type::None;
        }
        else
        {
            if (FileSystem::Instance()->ReadFileContents(collectedFile.absPath, origFileBuffer) == false)
            {
                LOG_ERROR("Can't read contents of %s", collectedFile.absPath.GetAbsolutePathname().c_str());
                return false;
            }

            bool useCompressedBuffer = (compressionType != Compressor::Type::None);
            if (useCompressedBuffer)
            {
                if (!compressor->Compress(origFileBuffer, compressedFileBuffer))
                {
                    LOG_ERROR("Can't compress contents of %s", collectedFile.absPath.GetAbsolutePathname().c_str());
                    return false;
                }
                useCompressedBuffer = (compressedFileBuffer.size() < origFileBuffer.size());
            }

            fileEntry.startPositionInPackedFilesBlock = dataOffset;
            fileEntry.original = origFileBuffer.size();
            fileEntry.compressed = compressedFileBuffer.size();
            fileEntry.packType = (useCompressedBuffer ? compressionType : Compressor::Type::None);

            Vector<uint8>& srcBuffer = (useCompressedBuffer ? compressedFileBuffer : origFileBuffer);
            dataOffset += srcBuffer.size();

            if (!WriteRawData(outputFile, srcBuffer))
            {
                return false;
            }
        }

        static String deviceName = WStringToString(DeviceInfo::GetName());
        DateTime dateTime = DateTime::Now();
        String date = WStringToString(dateTime.GetLocalizedDate());
        String time = WStringToString(dateTime.GetLocalizedTime());
        Logger::Info("%s | %s %s | Packed %s, orig size %u, compressed size %u, compression: %s",
                     deviceName.c_str(), date.c_str(), time.c_str(),
                     collectedFile.archivePath.c_str(), fileEntry.original, fileEntry.compressed,
                     CompressTypeToString(fileEntry.packType).c_str());
    }

    outputFile->Seek(headerBlock.startFilesDataBlockPosition, File::SEEK_FROM_START);
    if (!WriteFilesDataBlock(outputFile, filesDataBlock))
    {
        return false;
    }

    return true;
}

bool CreateArchive(const Vector<String>& sources, bool addHiddenFiles, DAVA::Compressor::Type compressionType,
                   const FilePath& archivePath, const FilePath& logPath, AssetCacheClient* assetCacheClient)
{
    Vector<CollectedFile> collectedFiles = CollectFiles(sources, addHiddenFiles);
    if (collectedFiles.empty())
    {
        LOG_ERROR("No input files for pack");
        return false;
    }

    if (archivePath.IsEmpty())
    {
        LOG_ERROR("Archive path is not set");
        return false;
    }

    AssetCache::CacheItemKey keyForArchive;
    AssetCache::CacheItemKey keyForLog;
    if (assetCacheClient != nullptr)
    {
        ConstructCacheKeys(keyForArchive, keyForLog, collectedFiles, CompressTypeToString(compressionType));

        if (true == RetrieveFromCache(assetCacheClient, keyForArchive, keyForLog, archivePath, logPath))
        {
            return true;
        }
    }

    if (Pack(collectedFiles, compressionType, archivePath))
    {
        Logger::Info("packing done");

        if (assetCacheClient != nullptr)
        {
            AddToCache(assetCacheClient, keyForArchive, keyForLog, archivePath, logPath);
        }
        return true;
    }
    else
    {
        Logger::Info("packing failed");
        return false;
    }
}

} // namespace Archiver
} // namespace DAVA
