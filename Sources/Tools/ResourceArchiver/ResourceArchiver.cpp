#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileList.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "Utils/Utils.h"
#include "Compression/LZ4Compressor.h"
#include "Compression/ZipCompressor.h"
#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"

#include "AssetCache/AssetCache.h"
#include "AssetCache/AssetCacheClient.h"

#include "ResourceArchiver/ResourceArchiver.h"

#include <algorithm>
#include <Utils/CRC32.h>

ENUM_DECLARE(DAVA::Compressor::Type)
{
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::Lz4), "lz4");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::Lz4HC), "lz4hc");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::RFC1951), "rfc1951");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::None), "none");
};

namespace DAVA
{
namespace ResourceArchiver
{
struct CollectedFile
{
    FilePath absPath;
    String archivePath;
};

void CollectAllFilesInDirectory(const FilePath& dirPath, const String& dirArchivePath, bool addHidden, Vector<CollectedFile>& collectedFiles)
{
    bool fileOrDirAdded = false;

    ScopedPtr<FileList> fileList(new FileList(dirPath, addHidden));
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
}

bool WriteHeaderBlock(File* outputFile, const PackFormat::PackFile::FooterBlock& footer)
{
    DVASSERT(outputFile != nullptr);

    uint32 sizeToWrite = sizeof(PackFormat::PackFile::FooterBlock);
    uint32 written = outputFile->Write(&footer, sizeToWrite);
    if (written != sizeToWrite)
    {
        Logger::Error("Can't write header block to archive");
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
        Logger::Error("Can't write data block to archive");
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
            Logger::Error("Can't export retrieved data to file %s", outputFile.GetAbsolutePathname().c_str());
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

bool CollectFiles(const Vector<String>& sources, bool addHiddenFiles, Vector<CollectedFile>& collectedFiles)
{
    for (String source : sources)
    {
        FilePath sourcePath(source);
        if (sourcePath.IsDirectoryPathname())
        {
            CollectAllFilesInDirectory(sourcePath, sourcePath.GetLastDirectoryName() + '/', addHiddenFiles, collectedFiles);
        }
        else
        {
            CollectedFile collectedFile;
            collectedFile.absPath = sourcePath;
            collectedFile.archivePath = sourcePath.GetFilename();
            collectedFiles.push_back(collectedFile);
        }
    }

    std::stable_sort(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                     {
                         return left.archivePath < right.archivePath;
                     });

    // removing duplicate files
    auto pointerAtDuplicates = std::unique(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                                           {
                                               if (left.absPath == right.absPath)
                                               {
                                                   Logger::Warning("Skipping duplicate %s", left.absPath.GetAbsolutePathname().c_str());
                                                   return true;
                                               }
                                               else
                                               {
                                                   return false;
                                               }
                                           });
    collectedFiles.erase(pointerAtDuplicates, collectedFiles.end());

    // check colliding files (they are different but have same archivePath and thus one will rewrite another during unpack)
    pointerAtDuplicates = std::unique(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                                      {
                                          if (left.archivePath == right.archivePath)
                                          {
                                              Logger::Error("'%s' and '%s' will be having the same path '%s' in archive",
                                                            left.absPath.GetAbsolutePathname().c_str(), right.absPath.GetAbsolutePathname().c_str(), right.archivePath.c_str());
                                              return true;
                                          }
                                          else
                                          {
                                              return false;
                                          }
                                      });
    if (pointerAtDuplicates != collectedFiles.end())
    {
        return false;
    }

    return true;
}

const Compressor* GetCompressor(Compressor::Type compressorType)
{
    static const std::unique_ptr<Compressor> lz4Compressor(new LZ4Compressor);
    static const std::unique_ptr<Compressor> lz4HCCompressor(new LZ4HCCompressor);
    static const std::unique_ptr<Compressor> zipCompressor(new ZipCompressor);

    switch (compressorType)
    {
    case Compressor::Type::Lz4:
        return lz4Compressor.get();
    case Compressor::Type::Lz4HC:
        return lz4HCCompressor.get();
    case Compressor::Type::RFC1951:
        return zipCompressor.get();
    default:
    {
        DVASSERT_MSG(false, Format("Unexpected compressor type: %u", compressorType).c_str());
        return nullptr;
    }
    }
}

bool Pack(const Vector<CollectedFile>& collectedFiles, DAVA::Compressor::Type compressionType, File* outputFile)
{
    PackFormat::PackFile packFile;

    Vector<uint8> origFileBuffer;
    Vector<uint8> compressedFileBuffer;

    const Compressor* compressor = nullptr;
    if (compressionType != Compressor::Type::None)
    {
        compressor = GetCompressor(compressionType);
        DVASSERT_MSG(compressor, Format("Can't get '%s' compressor", GlobalEnumMap<Compressor::Type>::Instance()->ToString(static_cast<int>(compressionType))).c_str());
    }

    uint64 dataOffset = 0;

    FileSystem* fs = FileSystem::Instance();

    try
    {
        std::for_each(begin(collectedFiles), end(collectedFiles), [&](const CollectedFile& f)
                      {
                          const CollectedFile& collectedFile = f;
                          PackFormat::FileTableEntry fileEntry = { 0 };

                          if (fs->ReadFileContents(collectedFile.absPath, origFileBuffer) == false)
                          {
                              throw std::runtime_error("Can't read contents of " + collectedFile.absPath.GetAbsolutePathname());
                          }

                          bool useCompressedBuffer = (compressionType != Compressor::Type::None && !origFileBuffer.empty());
                          if (useCompressedBuffer)
                          {
                              if (!compressor->Compress(origFileBuffer, compressedFileBuffer))
                              {
                                  throw std::runtime_error("Can't compress contents of " + collectedFile.absPath.GetAbsolutePathname());
                              }
                              useCompressedBuffer = (compressedFileBuffer.size() < origFileBuffer.size());
                          }

                          Vector<uint8>& useBuffer = (useCompressedBuffer ? compressedFileBuffer : origFileBuffer);

                          fileEntry.startPosition = dataOffset;
                          fileEntry.originalSize = static_cast<uint32>(origFileBuffer.size());
                          fileEntry.compressedSize = static_cast<uint32>(compressedFileBuffer.size());
                          fileEntry.type = (useCompressedBuffer ? compressionType : Compressor::Type::None);
                          fileEntry.compressedCrc32 = CRC32::ForBuffer(useBuffer.data(), useBuffer.size());
                          fileEntry.originalCrc32 = CRC32::ForBuffer(origFileBuffer.data(), origFileBuffer.size());
                          fileEntry.reserved.fill(0); // do it or your crc32 randomly change on same files

                          dataOffset += static_cast<uint32>(useBuffer.size());

                          if (!WriteRawData(outputFile, useBuffer))
                          {
                              throw std::runtime_error("can't write buffer to output file");
                          }

                          packFile.filesTable.data.files.push_back(fileEntry);

                          static String deviceName = WStringToString(DeviceInfo::GetName());
                          DateTime dateTime = DateTime::Now();
                          String date = WStringToString(dateTime.GetLocalizedDate());
                          String time = WStringToString(dateTime.GetLocalizedTime());
                          Logger::Info("%s | %s %s | Packed %s, orig size %u, compressed size %u, compression: %s, crc32: 0x%X",
                                       deviceName.c_str(), date.c_str(), time.c_str(),
                                       collectedFile.archivePath.c_str(), fileEntry.originalSize, fileEntry.compressedSize,
                                       GlobalEnumMap<Compressor::Type>::Instance()->ToString(static_cast<int>(fileEntry.type)), fileEntry.compressedCrc32);

                          compressedFileBuffer.clear();
                      });
    }
    catch (std::exception& ex)
    {
        Logger::Error("%s", ex.what());
        return false;
    }

    PackFormat::PackFile::FooterBlock& footerBlock = packFile.footer;
    PackFormat::PackFile::FilesTableBlock::Names& namesBlock = packFile.filesTable.names;
    PackFormat::PackFile::FilesTableBlock::FilesData& filesDataBlock = packFile.filesTable.data;

    Vector<uint8> namesOriginal = SpliceFileNames(collectedFiles);
    namesBlock.compressedNames.reserve(namesOriginal.size());

    if (!GetCompressor(Compressor::Type::Lz4HC)->Compress(namesOriginal, namesBlock.compressedNames))
    {
        Logger::Error("Can't compress names block");
        return false;
    }

    namesBlock.compressedCrc32 = CRC32::ForBuffer(reinterpret_cast<char*>(namesBlock.compressedNames.data()),
                                                  static_cast<uint32>(namesBlock.compressedNames.size()));

    uint32 fileDataSize = static_cast<uint32>(filesDataBlock.files.size() * sizeof(PackFormat::FileTableEntry));
    uint32 fileTableSize = fileDataSize + static_cast<uint32>(namesBlock.compressedNames.size() + sizeof(namesBlock.compressedCrc32));

    // place in one buffer full FileTableBlock first all filesData then compressed names and compressedNamesCrc32
    Vector<uint8> tmpFileTable;
    tmpFileTable.resize(fileTableSize);

    // copy files data
    std::copy_n(filesDataBlock.files.data(), filesDataBlock.files.size(), reinterpret_cast<PackFormat::FileTableEntry*>(tmpFileTable.data()));

    // copy compressed names data
    std::copy_n(namesBlock.compressedNames.data(), namesBlock.compressedNames.size(), &tmpFileTable[fileDataSize]);

    // copy compressed crc32
    std::copy_n(&namesBlock.compressedCrc32, 1, reinterpret_cast<uint32*>(&tmpFileTable[fileDataSize - sizeof(namesBlock.compressedCrc32)]));

    footerBlock.info.filesTableSize = fileTableSize;
    footerBlock.info.filesTableCrc32 = CRC32::ForBuffer(reinterpret_cast<char*>(tmpFileTable.data()), static_cast<uint32>(tmpFileTable.size()));
    footerBlock.info.packArchiveMarker = PackFormat::FileMarker;
    footerBlock.info.numFiles = static_cast<uint32>(filesDataBlock.files.size());
    footerBlock.info.namesSizeCompressed = static_cast<uint32>(namesBlock.compressedNames.size());
    footerBlock.info.namesSizeOriginal = static_cast<uint32>(namesOriginal.size());

    footerBlock.infoCrc32 = CRC32::ForBuffer(reinterpret_cast<char*>(&footerBlock.info), sizeof(footerBlock.info));
    footerBlock.reserved.fill(0);

    uint32 numBytesWrite = outputFile->Write(tmpFileTable.data(), static_cast<uint32>(tmpFileTable.size()));
    if (numBytesWrite != tmpFileTable.size())
    {
        Logger::Error("Can't write fileTableBlock");
        return false;
    }

    if (!WriteHeaderBlock(outputFile, footerBlock))
    {
        Logger::Error("Can't write footerBlock");
        return false;
    }

    return true;
}

bool Pack(const Vector<CollectedFile>& collectedFiles, DAVA::Compressor::Type compressionType, const FilePath& archivePath)
{
    ScopedPtr<File> outputFile(File::Create(archivePath, File::CREATE | File::WRITE));
    if (!outputFile)
    {
        Logger::Error("Can't create %s", archivePath.GetAbsolutePathname().c_str());
        return false;
    }

    bool packWasSuccessfull = Pack(collectedFiles, compressionType, outputFile);
    outputFile.reset();

    if (packWasSuccessfull)
    {
        return true;
    }
    else
    {
        if (!FileSystem::Instance()->DeleteFile(archivePath))
        {
            Logger::Error("Can't delete %s", archivePath.GetAbsolutePathname().c_str());
        }
        return false;
    }
}

bool CreateArchive(const Params& params)
{
    Vector<CollectedFile> collectedFiles;
    if (false == CollectFiles(params.sourcesList, params.addHiddenFiles, collectedFiles))
    {
        Logger::Error("Collecting files error");
        return false;
    }

    if (collectedFiles.empty())
    {
        Logger::Error("No input files for pack");
        return false;
    }

    if (params.archivePath.IsEmpty())
    {
        Logger::Error("Archive path is not set");
        return false;
    }

    AssetCache::CacheItemKey keyForArchive;
    AssetCache::CacheItemKey keyForLog;
    if (params.assetCacheClient != nullptr)
    {
        const char* compressionStr = GlobalEnumMap<Compressor::Type>::Instance()->ToString(static_cast<int>(params.compressionType));
        ConstructCacheKeys(keyForArchive, keyForLog, collectedFiles, compressionStr);

        if (true == RetrieveFromCache(params.assetCacheClient, keyForArchive, keyForLog, params.archivePath, params.logPath))
        {
            return true;
        }
    }

    if (Pack(collectedFiles, params.compressionType, params.archivePath))
    {
        Logger::Info("packing done");

        if (params.assetCacheClient != nullptr)
        {
            AddToCache(params.assetCacheClient, keyForArchive, keyForLog, params.archivePath, params.logPath);
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
