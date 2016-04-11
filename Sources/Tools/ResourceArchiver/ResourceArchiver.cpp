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

#include "ResourceArchiver/ResourceArchiver.h"

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

bool StringToPackType(const String& compressionStr, Compressor::Type& type)
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

String PackTypeToString(Compressor::Type packType)
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

// static bool Packing(Vector<PackFormat::FileTableEntry>& fileTable,
//                     const Vector<uint8>& inBuffer,
//                     Vector<uint8>& packingBuf,
//                     File* output,
//                     uint32 startPos,
//                     Compressor::Type packingType)
// {
//     bool result = false;
//
//     if (packingType == Compressor::Type::Lz4HC)
//     {
//         result = LZ4HCCompressor().Compress(inBuffer, packingBuf);
//     }
//     else if (packingType == Compressor::Type::Lz4)
//     {
//         result = LZ4Compressor().Compress(inBuffer, packingBuf);
//     }
//     else if (packingType == Compressor::Type::RFC1951)
//     {
//         result = ZipCompressor().Compress(inBuffer, packingBuf);
//     }
//
//     if (!result)
//     {
//         return false;
//     }
//
//     uint32 packedSize = static_cast<uint32>(packingBuf.size());
//
//     // if packed size worse then raw leave raw bytes
//     uint32 writeOk = 0;
//     if (packedSize >= inBuffer.size())
//     {
//         packedSize = static_cast<uint32>(inBuffer.size());
//         packingType = Compressor::Type::None;
//         writeOk = output->Write(&inBuffer[0], packedSize);
//     }
//     else
//     {
//         writeOk = output->Write(&packingBuf[0], packedSize);
//     }
//
//     if (writeOk == 0)
//     {
//         Logger::Error("can't write into tmp archive file");
//         return false;
//     }
//
//     PackFormat::PackFile::FilesDataBlock::Data fileData{
//         startPos, packedSize, static_cast<uint32>(inBuffer.size()), packingType
//     };
//
//     fileTable.push_back(fileData);
//
//     return result;
// }
//
// bool CompressFileAndWriteToOutput(const String&
//                                     const FilePath& baseFolder,
//                                     Vector<PackFormat::FileTableEntry>& fileTable,
//                                     Vector<uint8>& inBuffer,
//                                     Vector<uint8>& packingBuf,
//                                     File* output)
// {
//     using namespace PackFormat;
//
//     if ()
//
//     String fullPath(baseFolder.GetAbsolutePathname() + fInfo.relativeFilePath);
//
//     RefPtr<File> inFile(File::Create(fullPath, File::OPEN | File::READ));
//
//     if (!inFile)
//     {
//         Logger::Error("can't read input file: %s\n", fInfo.relativeFilePath);
//         return false;
//     }
//
//     if (!inFile->Seek(0, File::SEEK_FROM_END))
//     {
//         Logger::Error("can't seek inside file: %s\n", fInfo.relativeFilePath);
//         return false;
//     }
//     uint32 origSize = inFile->GetPos();
//     if (!inFile->Seek(0, File::SEEK_FROM_START))
//     {
//         Logger::Error("can't seek inside file: %s\n", fInfo.relativeFilePath);
//         return false;
//     }
//
//     uint32 startPos{ 0 };
//     if (fileTable.empty() == false)
//     {
//         auto& prevPackData = fileTable.back();
//         startPos = prevPackData.startPositionInPackedFilesBlock + prevPackData.compressed;
//     }
//
//     if (origSize > 0)
//     {
//         inBuffer.resize(origSize);
//         uint32 readOk = inFile->Read(&inBuffer[0], origSize);
//         if (readOk <= 0)
//         {
//             Logger::Error("can't read input file: %s\n", fInfo.relativeFilePath);
//             return false;
//         }
//     }
//     else
//     {
//         PackFile::FilesDataBlock::Data fileData{
//             startPos,
//             origSize,
//             origSize,
//             Compressor::Type::None
//         };
//
//         fileTable.push_back(fileData);
//         return true;
//     }
//
//     Compressor::Type compressType = fInfo.compressionType;
//
//     if (fInfo.compressionType == Compressor::Type::None)
//     {
//         PackFile::FilesDataBlock::Data fileData{
//             startPos,
//             origSize,
//             origSize,
//             Compressor::Type::None
//         };
//
//         fileTable.push_back(fileData);
//         bool writeOk = (output->Write(&inBuffer[0], origSize) == origSize);
//         if (writeOk)
//         {
//             Logger::Error("can't write into tmp archive file");
//             return false;
//         }
//     }
//     else
//     {
//         if (!Packing(fileTable, inBuffer, packingBuf, output, startPos, fInfo.compressionType))
//         {
//             return false;
//         }
//     }
//     return true;
// }

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

bool CreateArchive(const FilePath& archiveName, const Vector<String>& sources, bool addHiddenFiles)
{
    Vector<CollectedFile> collectedFiles;

    for (const String& source : sources)
    {
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

    if (collectedFiles.empty())
    {
        LOG_ERROR("No input files for pack");
        return false;
    }

    std::stable_sort(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                     {
                         return CompareCaseInsensitive(left.archivePath, right.archivePath) < 0;
                     });

    PackFormat::PackFile packFile;
    PackFormat::PackFile::HeaderBlock& headerBlock = packFile.header;
    PackFormat::PackFile::NamesBlock& namesBlock = packFile.names;
    PackFormat::PackFile::FilesDataBlock& filesDataBlock = packFile.filesData;

    auto& fileTable = filesDataBlock.files;
    fileTable.resize(collectedFiles.size());

    headerBlock.marker = PackFormat::FileMarker;
    headerBlock.numFiles = static_cast<uint32>(fileTable.size());

    StringStream ss;
    for (const CollectedFile& file : collectedFiles)
    {
        ss << file.archivePath << '\0';
    }
    String strNames(ss.str());
    Vector<uint8> sortedNamesOriginal(strNames.begin(), strNames.end());

    namesBlock.sortedNamesLz4hc.reserve(sortedNamesOriginal.size());

    if (!LZ4HCCompressor().Compress(sortedNamesOriginal, namesBlock.sortedNamesLz4hc))
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

    UniquePtr<File> outputFile(File::Create(archiveName, File::CREATE | File::WRITE));
    if (!outputFile)
    {
        LOG_ERROR("Can't create %s", archiveName.GetAbsolutePathname().c_str());
        return false;
    }

    if (!WriteHeaderBlock(outputFile, headerBlock) || !WriteNamesBlock(outputFile, namesBlock) || !WriteFilesDataBlock(outputFile, filesDataBlock))
    {
        return false;
    }

    uint32 dataOffset = headerBlock.startPackedFilesBlockPosition;

    Vector<uint8> origFileBuffer;
    Vector<uint8> compressedFileBuffer;

    for (size_t i = 0, filesSize = collectedFiles.size(); i < filesSize; ++i)
    {
        CollectedFile& collectedFile = collectedFiles[i];
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

            if (!LZ4HCCompressor().Compress(origFileBuffer, compressedFileBuffer))
            {
                LOG_ERROR("Can't compress contents of %s", collectedFile.absPath.GetAbsolutePathname().c_str());
                return false;
            }

            bool useOriginalBuffer = (compressedFileBuffer.size() >= origFileBuffer.size());

            fileEntry.startPositionInPackedFilesBlock = dataOffset;
            fileEntry.original = origFileBuffer.size();
            fileEntry.compressed = compressedFileBuffer.size();
            fileEntry.packType = (useOriginalBuffer ? Compressor::Type::None : Compressor::Type::Lz4HC);

            Vector<uint8>& srcBuffer = (useOriginalBuffer ? origFileBuffer : compressedFileBuffer);
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
                     PackTypeToString(fileEntry.packType).c_str());
    }

    outputFile->Seek(headerBlock.startFilesDataBlockPosition, File::SEEK_FROM_START);
    if (!WriteFilesDataBlock(outputFile, filesDataBlock))
    {
        return false;
    }

    return true;
}

// bool CreateArchive(const FilePath& pacName,
//                 const Vector<String>& sortedFileNames,
//                 void (*onPackOneFile)(const ResourceArchive::FileInfo&))
// {
//
// }

// void CollectAllFilesInDirectory(const String& pathDirName,
//                                 bool includeHidden,
//                                 Vector<String>& output)
// {
//     FilePath pathToDir = pathDirName;
//     RefPtr<FileList> fileList(new FileList(pathToDir, includeHidden));
//     for (auto file = 0; file < fileList->GetCount(); ++file)
//     {
//         if (fileList->IsDirectory(file))
//         {
//             auto directoryName = fileList->GetFilename(file);
//             if ((directoryName != "..") && (directoryName != "."))
//             {
//                 String subDir = pathDirName == "."
//                 ?
//                 directoryName + '/'
//                 :
//                 pathDirName + directoryName + '/';
//                 CollectAllFilesInDirectory(subDir, includeHidden, output);
//             }
//         }
//         else
//         {
//             String filename = fileList->GetFilename(file);
//             if (filename.at(0) == '.' && !includeHidden)
//             {
//                 continue;
//             }
//             String pathname =
//             (pathDirName == "." ? filename : pathDirName + filename);
//             output.push_back(pathname);
//         }
//     }
// }

// Compressor::Type ToPackType(const String& value, Compressor::Type defaultVal)
// {
//     const Vector<std::pair<String, Compressor::Type>>
//     packTypes = { { "lz4", Compressor::Type::Lz4 },
//                   { "lz4hc", Compressor::Type::Lz4HC },
//                   { "none", Compressor::Type::None } };
//     for (auto& pair : packTypes)
//     {
//         if (pair.first == value)
//         {
//             return pair.second;
//         }
//     }
//     std::cerr << "can't convert: \"" << value
//               << "\" into any valid compression type, use default\n";
//     return defaultVal;
// }

// String ToString(Compressor::Type packType)
// {
//     static const Vector<const char*> packTypes = { "none", "lz4", "lz4hc", "rfc1951" };
//
//     size_t index = static_cast<size_t>(packType);
//
//     return packTypes.at(index);
// }
//
// void OnOneFilePacked(const ResourceArchive::FileInfo& info)
// {
//     std::cout << "packing file: " << info.relativeFilePath
//               << " compressed: " << info.compressedSize
//               << " original: " << info.originalSize
//               << " packingType: " << ToString(info.compressionType) << '\n';
// }
//
// int PackDirectory(const String& dir,
//                   const String& pakfileName,
//                   bool includeHidden)
// {
//     std::cout << "===================================================\n"
//               << "=== Packer started\n"
//               << "=== Pack directory: " << dir << '\n'
//               << "=== Pack archiveName: " << pakfileName << '\n'
//               << "===================================================\n";
//
//     auto dirWithSlash = (dir.back() == '/' ? dir : dir + '/');
//
//     RefPtr<FileSystem> fileSystem(new FileSystem());
//
//     FilePath absPathPack =
//     fileSystem->GetCurrentWorkingDirectory() + pakfileName;
//
//     if (!fileSystem->SetCurrentWorkingDirectory(dirWithSlash))
//     {
//         std::cerr << "can't set CWD to: " << dirWithSlash << '\n';
//         return EXIT_FAILURE;
//     }
//
//     Vector<String> files;
//
//     CollectAllFilesInDirectory(".", includeHidden, files);
//
//     if (files.empty())
//     {
//         std::cerr << "no files found in: " << dir << '\n';
//         return EXIT_FAILURE;
//     }
//
//     if (fileSystem->IsFile(absPathPack))
//     {
//         std::cerr << "pakfile already exist! Delete it.\n";
//         if (0 != std::remove(absPathPack.GetAbsolutePathname().c_str()))
//         {
//             std::cerr << "can't remove existing pakfile.\n";
//             return EXIT_FAILURE;
//         }
//     }
//
//     std::stable_sort(begin(files), end(files));
//
//     Vector<ResourceArchive::FileInfo> fInfos;
//     fInfos.reserve(files.size());
//
//     for (auto& f : files)
//     {
//         ResourceArchive::FileInfo info;
//         info.relativeFilePath = f.c_str();
//         info.compressedSize = 0;
//         info.originalSize = 0;
//         info.compressionType = Compressor::Type::Lz4HC; // TODO change what you need
//         fInfos.push_back(info);
//     }
//
//     FilePath baseDir;
//
//     std::cout << "start packing...\n";
//     if (Create(absPathPack, baseDir, fInfos, OnOneFilePacked))
//     {
//         std::cout << "Success!\n";
//         return EXIT_SUCCESS;
//     }
//     else
//     {
//         std::cout << "Failed!\n";
//         return EXIT_FAILURE;
//     }
// }

// bool UnpackFile(const ResourceArchive& ra,
//                 const ResourceArchive::FileInfo& fileInfo)
// {
//     Vector<uint8> file;
//     if (!ra.LoadFile(fileInfo.relativeFilePath, file))
//     {
//         std::cerr << "can't load file: " << fileInfo.relativeFilePath << " from archive\n";
//         return false;
//     }
//     String filePath(fileInfo.relativeFilePath);
//     FilePath fullPath = filePath;
//     FilePath dirPath = fullPath.GetDirectory();
//     FileSystem::eCreateDirectoryResult result =
//     FileSystem::Instance()->CreateDirectory(dirPath, true);
//     if (FileSystem::DIRECTORY_CANT_CREATE == result)
//     {
//         std::cerr << "can't create unpack path dir: "
//                   << dirPath.GetAbsolutePathname() << '\n';
//         return false;
//     }
//     RefPtr<File> f(File::Create(fullPath, File::CREATE | File::WRITE));
//     if (!f)
//     {
//         std::cerr << "can't create file: " << fileInfo.relativeFilePath << '\n';
//         return false;
//     }
//     uint32 writeOk = f->Write(file.data(), file.size());
//     if (writeOk < file.size())
//     {
//         std::cerr << "can't write into file: " << fileInfo.relativeFilePath << '\n';
//         return false;
//     }
//     return true;
// }
//
// int UnpackToDirectory(const String& pak, const String& dir)
// {
//     RefPtr<FileSystem> fs(new FileSystem());
//     if (!fs)
//     {
//         std::cerr << "can't create FileSystem\n";
//         return EXIT_FAILURE;
//     }
//     String cwd = fs->GetCurrentWorkingDirectory().GetAbsolutePathname();
//
//     std::cout << "===================================================\n"
//               << "=== Unpacker started\n"
//               << "=== Unpack directory: " << dir << '\n'
//               << "=== Unpack archiveName: " << pak << '\n'
//               << "===================================================\n";
//
//     fs->CreateDirectory(dir);
//     if (!fs->SetCurrentWorkingDirectory(dir + '/'))
//     {
//         std::cerr << "can't set CWD to " << dir << '\n';
//         return EXIT_FAILURE;
//     }
//
//     String absPathPackFile = cwd + pak;
//     std::unique_ptr<ResourceArchive> ra;
//
//     try
//     {
//         ra.reset(new ResourceArchive(absPathPackFile));
//     }
//     catch (std::exception& ex)
//     {
//         std::cerr << "can't open archive: " << ex.what() << '\n';
//         return EXIT_FAILURE;
//     }
//
//     for (auto& fileInfo : ra->GetFilesInfo())
//     {
//         std::cout << "start unpacking: " << fileInfo.relativeFilePath
//                   << " compressed: " << fileInfo.compressedSize
//                   << " original: " << fileInfo.originalSize
//                   << " packingType: " << ToString(fileInfo.compressionType)
//                   << '\n';
//         if (!UnpackFile(*ra, fileInfo))
//         {
//             return EXIT_FAILURE;
//         }
//     }
//     std::cout << "Success!\n";
//     return EXIT_SUCCESS;
// }
//
// int ListContent(const String& pakfile)
// {
//     RefPtr<FileSystem> fs(new FileSystem());
//     if (!fs)
//     {
//         std::cerr << "can't create FileSystem\n";
//         return EXIT_FAILURE;
//     }
//     std::unique_ptr<ResourceArchive> ra;
//     try
//     {
//         ra.reset(new ResourceArchive(pakfile));
//     }
//     catch (...)
//     {
//         return EXIT_FAILURE;
//     }
//
//     for (auto& info : ra->GetFilesInfo())
//     {
//         std::cout << info.relativeFilePath << " compressed: " << info.compressedSize
//                   << " original: " << info.originalSize
//                   << " type: " << ToString(info.compressionType) << '\n';
//     }
//     return EXIT_SUCCESS;
// }
//
// static Compressor::Type GetPackingBestType(const String& fileName)
// {
//     return Compressor::Type::Lz4HC;
// };

// int main(int argc, char* argv[])
// {
//     ProgramOptions packOptions("pack");
//     packOptions.AddOption("--compression", VariantType(String("lz4hc")),
//                           "default compression method, lz4hc - default");
//     packOptions.AddOption("--add_hidden", VariantType(String("false")),
//                           "add hidden files to pack list (false or true)");
//     // default rule pack all files into lz4hc
//     packOptions.AddOption("--rule", VariantType(String(".lz4hc")),
//                           "rule to select compression type like: --rule "
//                           "xml.lz4 supported lz4, lz4hc, none",
//                           true);
//     packOptions.AddArgument("directory");
//     packOptions.AddArgument("pakfile");
//
//     ProgramOptions unpackOptions("unpack");
//     unpackOptions.AddArgument("pakfile");
//     unpackOptions.AddArgument("directory");
//
//     ProgramOptions listOptions("list");
//     listOptions.AddArgument("pakfile");
//
//     if (packOptions.Parse(argc, argv))
//     {
//         auto dirName = packOptions.GetArgument("directory");
//         auto pakFile = packOptions.GetArgument("pakfile");
//         auto addHidden = packOptions.GetOption("--add_hidden").AsString();
//
//         return PackDirectory(dirName, pakFile, addHidden == "true");
//     }
//
//     if (unpackOptions.Parse(argc, argv))
//     {
//         auto pakFile = unpackOptions.GetArgument("pakfile");
//         auto dirName = unpackOptions.GetArgument("directory");
//
//         return UnpackToDirectory(pakFile, dirName);
//     }
//
//     if (listOptions.Parse(argc, argv))
//     {
//         auto pakFile = listOptions.GetArgument("pakfile");
//         return ListContent(pakFile);
//     }
//
//     {
//         packOptions.PrintUsage();
//         unpackOptions.PrintUsage();
//         listOptions.PrintUsage();
//         return EXIT_FAILURE;
//     }
// }

} // namespace Archiver
} // namespace DAVA
