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

namespace DAVA
{
namespace Archiver
{
static bool Packing(Vector<PackFormat::FileTableEntry>& fileTable,
                    const Vector<uint8>& inBuffer,
                    Vector<uint8>& packingBuf,
                    File* output,
                    uint32 startPos,
                    Compressor::Type packingType)
{
    bool result = false;

    if (packingType == Compressor::Type::Lz4HC)
    {
        result = LZ4HCCompressor().Compress(inBuffer, packingBuf);
    }
    else if (packingType == Compressor::Type::Lz4)
    {
        result = LZ4Compressor().Compress(inBuffer, packingBuf);
    }
    else if (packingType == Compressor::Type::RFC1951)
    {
        result = ZipCompressor().Compress(inBuffer, packingBuf);
    }

    if (!result)
    {
        return false;
    }

    uint32 packedSize = static_cast<uint32>(packingBuf.size());

    // if packed size worse then raw leave raw bytes
    uint32 writeOk = 0;
    if (packedSize >= inBuffer.size())
    {
        packedSize = static_cast<uint32>(inBuffer.size());
        packingType = Compressor::Type::None;
        writeOk = output->Write(&inBuffer[0], packedSize);
    }
    else
    {
        writeOk = output->Write(&packingBuf[0], packedSize);
    }

    if (writeOk == 0)
    {
        Logger::Error("can't write into tmp archive file");
        return false;
    }

    PackFormat::PackFile::FilesDataBlock::Data fileData{
        startPos, packedSize, static_cast<uint32>(inBuffer.size()), packingType
    };

    fileTable.push_back(fileData);

    return result;
}

static bool CompressFileAndWriteToOutput(const ResourceArchive::FileInfo& fInfo,
                                         const FilePath& baseFolder,
                                         Vector<PackFormat::FileTableEntry>& fileTable,
                                         Vector<uint8>& inBuffer,
                                         Vector<uint8>& packingBuf,
                                         File* output)
{
    using namespace PackFormat;

    String fullPath(baseFolder.GetAbsolutePathname() + fInfo.relativeFilePath);

    RefPtr<File> inFile(File::Create(fullPath, File::OPEN | File::READ));

    if (!inFile)
    {
        Logger::Error("can't read input file: %s\n", fInfo.relativeFilePath);
        return false;
    }

    if (!inFile->Seek(0, File::SEEK_FROM_END))
    {
        Logger::Error("can't seek inside file: %s\n", fInfo.relativeFilePath);
        return false;
    }
    uint32 origSize = inFile->GetPos();
    if (!inFile->Seek(0, File::SEEK_FROM_START))
    {
        Logger::Error("can't seek inside file: %s\n", fInfo.relativeFilePath);
        return false;
    }

    uint32 startPos{ 0 };
    if (fileTable.empty() == false)
    {
        auto& prevPackData = fileTable.back();
        startPos = prevPackData.startPositionInPackedFilesBlock + prevPackData.compressed;
    }

    if (origSize > 0)
    {
        inBuffer.resize(origSize);
        uint32 readOk = inFile->Read(&inBuffer[0], origSize);
        if (readOk <= 0)
        {
            Logger::Error("can't read input file: %s\n", fInfo.relativeFilePath);
            return false;
        }
    }
    else
    {
        PackFile::FilesDataBlock::Data fileData{
            startPos,
            origSize,
            origSize,
            Compressor::Type::None
        };

        fileTable.push_back(fileData);
        return true;
    }

    Compressor::Type compressType = fInfo.compressionType;

    if (fInfo.compressionType == Compressor::Type::None)
    {
        PackFile::FilesDataBlock::Data fileData{
            startPos,
            origSize,
            origSize,
            Compressor::Type::None
        };

        fileTable.push_back(fileData);
        bool writeOk = (output->Write(&inBuffer[0], origSize) == origSize);
        if (writeOk)
        {
            Logger::Error("can't write into tmp archive file");
            return false;
        }
    }
    else
    {
        if (!Packing(fileTable, inBuffer, packingBuf, output, startPos, fInfo.compressionType))
        {
            return false;
        }
    }
    return true;
}

static bool CopyTmpfileToPackfile(RefPtr<File> packFileOutput,
                                  RefPtr<File> packedFile)
{
    Array<char, 4096> copyBuf;
    uint32 lastRead = packedFile->Read(&copyBuf[0], static_cast<uint32>(copyBuf.size()));
    while (lastRead == copyBuf.size())
    {
        uint32 lastWrite = packFileOutput->Write(&copyBuf[0], static_cast<uint32>(copyBuf.size()));
        if (lastWrite != copyBuf.size())
        {
            Logger::Error("can't write part of tmp archive file into output packfile\n");
            return false;
        }
        lastRead = packedFile->Read(&copyBuf[0], static_cast<uint32>(copyBuf.size()));
    }
    if (lastRead > 0)
    {
        uint32 lastWrite = packFileOutput->Write(&copyBuf[0], lastRead);
        if (lastWrite != lastRead)
        {
            Logger::Error("can't write part of tmp archive file into output packfile\n");
            return false;
        }
    }

    if (!packedFile->IsEof())
    {
        Logger::Error("can't read full content of tmp compressed output file\n");
        return false;
    }
    return true;
}

bool CreateArchive(const FilePath& archiveName,
                   const Vector<String>& srcFilenames,
                   void (*onPackOneFile)(const ResourceArchive::FileInfo&))
{
    const FilePath baseFolder = archiveName.GetDirectory();

    using namespace PackFormat;

    std::stable_sort(begin(infos), end(infos), [](const ResourceArchive::FileInfo& left, const ResourceArchive::FileInfo& right) -> bool
                     {
                         return std::strcmp(left.relativeFilePath, right.relativeFilePath) <= 0;
                     });

    RefPtr<File> packFileOutput(File::Create(archiveName, File::CREATE | File::WRITE));

    if (!packFileOutput)
    {
        Logger::Error("can't create packfile, can't Open: %s\n", archiveName.GetAbsolutePathname().c_str());
        return false;
    }

    PackFile pack;

    auto& fileTable = pack.filesData.files;

    Vector<uint8> fileBuffer;
    Vector<uint8> compressedFileBuffer;

    auto packedFileTmp = archiveName.GetAbsolutePathname() + "_tmp_compressed_files.bin";

    ScopedPtr<File> outTmpFile(File::Create(packedFileTmp, File::CREATE | File::WRITE));
    SCOPE_EXIT
    {
        FileSystem::Instance()->DeleteFile(packedFileTmp);
    };

    if (!outTmpFile)
    {
        Logger::Error("can't create tmp file for resource archive");
        return false;
    }

    pack.filesData.files.reserve(infos.size());

    for (const ResourceArchive::FileInfo& fileInfo : infos)
    {
        bool is_ok = CompressFileAndWriteToOutput(fileInfo, baseFolder, fileTable, fileBuffer, compressedFileBuffer, outTmpFile.get());
        if (!is_ok)
        {
            Logger::Info("can't pack file: %s, skip it\n", fileInfo.relativeFilePath);
            return false;
        }
        else if (onPackOneFile != nullptr)
        {
            FileTableEntry& last = fileTable.back();

            ResourceArchive::FileInfo info;

            info.relativeFilePath = fileInfo.relativeFilePath;
            info.originalSize = last.original;
            info.compressedSize = last.compressed;
            info.compressionType = last.packType;

            onPackOneFile(info);
        }
    };
    outTmpFile->Flush();

    PackFile::HeaderBlock& headerBlock = pack.header;
    headerBlock.marker = FileMarker;
    headerBlock.numFiles = static_cast<uint32>(infos.size());

    StringStream ss;
    std::for_each(begin(infos), end(infos),
                  [&ss](const ResourceArchive::FileInfo& fInfo)
                  {
                      ss << fInfo.relativeFilePath << '\0';
                  });

    String sortedNamesOriginal = std::move(ss.str());

    Vector<uint8> inputNames(begin(sortedNamesOriginal), end(sortedNamesOriginal));
    Vector<uint8> compressedNamesLz4;
    compressedNamesLz4.reserve(inputNames.size());

    if (!LZ4HCCompressor().Compress(inputNames, compressedNamesLz4))
    {
        Logger::Error("can't compress names block");
        return false;
    }

    pack.names.sortedNamesLz4hc = String(begin(compressedNamesLz4), end(compressedNamesLz4));

    headerBlock.namesBlockSizeCompressedLZ4HC = static_cast<uint32>(pack.names.sortedNamesLz4hc.size());
    headerBlock.namesBlockSizeOriginal = static_cast<uint32>(sortedNamesOriginal.size());

    headerBlock.startNamesBlockPosition = sizeof(PackFile::HeaderBlock);

    headerBlock.startFilesDataBlockPosition = headerBlock.startNamesBlockPosition + headerBlock.namesBlockSizeCompressedLZ4HC;

    uint32 sizeOfFilesTable = static_cast<uint32>(pack.filesData.files.size() *
                                                  sizeof(pack.filesData.files[0]));

    headerBlock.filesTableBlockSize = sizeOfFilesTable;
    headerBlock.startPackedFilesBlockPosition = headerBlock.startFilesDataBlockPosition + sizeOfFilesTable;

    uint32 delta = headerBlock.startPackedFilesBlockPosition;

    if (fileTable.empty())
    {
        Logger::Error("no input files for pack");
        return false;
    }

    for (auto& fileData : fileTable)
    {
        fileData.startPositionInPackedFilesBlock += delta;
    }

    uint32 writeOk = packFileOutput->Write(&pack.header, sizeof(pack.header));
    if (writeOk != sizeof(pack.header))
    {
        Logger::Error("can't write header block to archive");
        return false;
    }

    const String& sortedNames = pack.names.sortedNamesLz4hc;

    writeOk = packFileOutput->Write(sortedNames.data(), static_cast<uint32>(sortedNames.size()));
    if (writeOk != sortedNames.size())
    {
        Logger::Error("can't write filenames block to archive");
        return false;
    }
    writeOk = packFileOutput->Write(&pack.filesData.files[0],
                                    sizeOfFilesTable);

    if (writeOk != sizeOfFilesTable)
    {
        Logger::Error("can't write file table block to archive");
        return false;
    }

    RefPtr<File> tmpfile(File::Create(packedFileTmp, File::OPEN | File::READ));
    if (!tmpfile)
    {
        Logger::Error("can't open compressed tmp file");
        return false;
    }

    if (!CopyTmpfileToPackfile(packFileOutput, tmpfile))
    {
        return false;
    }

    tmpfile.Set(nullptr);

    return true;
}

// bool CreateArchive(const FilePath& pacName,
//                 const Vector<String>& sortedFileNames,
//                 void (*onPackOneFile)(const ResourceArchive::FileInfo&))
// {
//
// }

void CollectAllFilesInDirectory(const String& pathDirName,
                                bool includeHidden,
                                Vector<String>& output)
{
    FilePath pathToDir = pathDirName;
    RefPtr<FileList> fileList(new FileList(pathToDir, includeHidden));
    for (auto file = 0; file < fileList->GetCount(); ++file)
    {
        if (fileList->IsDirectory(file))
        {
            auto directoryName = fileList->GetFilename(file);
            if ((directoryName != "..") && (directoryName != "."))
            {
                String subDir = pathDirName == "."
                ?
                directoryName + '/'
                :
                pathDirName + directoryName + '/';
                CollectAllFilesInDirectory(subDir, includeHidden, output);
            }
        }
        else
        {
            String filename = fileList->GetFilename(file);
            if (filename.at(0) == '.' && !includeHidden)
            {
                continue;
            }
            String pathname =
            (pathDirName == "." ? filename : pathDirName + filename);
            output.push_back(pathname);
        }
    }
}

Compressor::Type ToPackType(const String& value, Compressor::Type defaultVal)
{
    const Vector<std::pair<String, Compressor::Type>>
    packTypes = { { "lz4", Compressor::Type::Lz4 },
                  { "lz4hc", Compressor::Type::Lz4HC },
                  { "none", Compressor::Type::None } };
    for (auto& pair : packTypes)
    {
        if (pair.first == value)
        {
            return pair.second;
        }
    }
    std::cerr << "can't convert: \"" << value
              << "\" into any valid compression type, use default\n";
    return defaultVal;
}

String ToString(Compressor::Type packType)
{
    static const Vector<const char*> packTypes = { "none", "lz4", "lz4hc", "rfc1951" };

    size_t index = static_cast<size_t>(packType);

    return packTypes.at(index);
}

void OnOneFilePacked(const ResourceArchive::FileInfo& info)
{
    std::cout << "packing file: " << info.relativeFilePath
              << " compressed: " << info.compressedSize
              << " original: " << info.originalSize
              << " packingType: " << ToString(info.compressionType) << '\n';
}

int PackDirectory(const String& dir,
                  const String& pakfileName,
                  bool includeHidden)
{
    std::cout << "===================================================\n"
              << "=== Packer started\n"
              << "=== Pack directory: " << dir << '\n'
              << "=== Pack archiveName: " << pakfileName << '\n'
              << "===================================================\n";

    auto dirWithSlash = (dir.back() == '/' ? dir : dir + '/');

    RefPtr<FileSystem> fileSystem(new FileSystem());

    FilePath absPathPack =
    fileSystem->GetCurrentWorkingDirectory() + pakfileName;

    if (!fileSystem->SetCurrentWorkingDirectory(dirWithSlash))
    {
        std::cerr << "can't set CWD to: " << dirWithSlash << '\n';
        return EXIT_FAILURE;
    }

    Vector<String> files;

    CollectAllFilesInDirectory(".", includeHidden, files);

    if (files.empty())
    {
        std::cerr << "no files found in: " << dir << '\n';
        return EXIT_FAILURE;
    }

    if (fileSystem->IsFile(absPathPack))
    {
        std::cerr << "pakfile already exist! Delete it.\n";
        if (0 != std::remove(absPathPack.GetAbsolutePathname().c_str()))
        {
            std::cerr << "can't remove existing pakfile.\n";
            return EXIT_FAILURE;
        }
    }

    std::stable_sort(begin(files), end(files));

    Vector<ResourceArchive::FileInfo> fInfos;
    fInfos.reserve(files.size());

    for (auto& f : files)
    {
        ResourceArchive::FileInfo info;
        info.relativeFilePath = f.c_str();
        info.compressedSize = 0;
        info.originalSize = 0;
        info.compressionType = Compressor::Type::Lz4HC; // TODO change what you need
        fInfos.push_back(info);
    }

    FilePath baseDir;

    std::cout << "start packing...\n";
    if (Create(absPathPack, baseDir, fInfos, OnOneFilePacked))
    {
        std::cout << "Success!\n";
        return EXIT_SUCCESS;
    }
    else
    {
        std::cout << "Failed!\n";
        return EXIT_FAILURE;
    }
}

bool UnpackFile(const ResourceArchive& ra,
                const ResourceArchive::FileInfo& fileInfo)
{
    Vector<uint8> file;
    if (!ra.LoadFile(fileInfo.relativeFilePath, file))
    {
        std::cerr << "can't load file: " << fileInfo.relativeFilePath << " from archive\n";
        return false;
    }
    String filePath(fileInfo.relativeFilePath);
    FilePath fullPath = filePath;
    FilePath dirPath = fullPath.GetDirectory();
    FileSystem::eCreateDirectoryResult result =
    FileSystem::Instance()->CreateDirectory(dirPath, true);
    if (FileSystem::DIRECTORY_CANT_CREATE == result)
    {
        std::cerr << "can't create unpack path dir: "
                  << dirPath.GetAbsolutePathname() << '\n';
        return false;
    }
    RefPtr<File> f(File::Create(fullPath, File::CREATE | File::WRITE));
    if (!f)
    {
        std::cerr << "can't create file: " << fileInfo.relativeFilePath << '\n';
        return false;
    }
    uint32 writeOk = f->Write(file.data(), file.size());
    if (writeOk < file.size())
    {
        std::cerr << "can't write into file: " << fileInfo.relativeFilePath << '\n';
        return false;
    }
    return true;
}

int UnpackToDirectory(const String& pak, const String& dir)
{
    RefPtr<FileSystem> fs(new FileSystem());
    if (!fs)
    {
        std::cerr << "can't create FileSystem\n";
        return EXIT_FAILURE;
    }
    String cwd = fs->GetCurrentWorkingDirectory().GetAbsolutePathname();

    std::cout << "===================================================\n"
              << "=== Unpacker started\n"
              << "=== Unpack directory: " << dir << '\n'
              << "=== Unpack archiveName: " << pak << '\n'
              << "===================================================\n";

    fs->CreateDirectory(dir);
    if (!fs->SetCurrentWorkingDirectory(dir + '/'))
    {
        std::cerr << "can't set CWD to " << dir << '\n';
        return EXIT_FAILURE;
    }

    String absPathPackFile = cwd + pak;
    std::unique_ptr<ResourceArchive> ra;

    try
    {
        ra.reset(new ResourceArchive(absPathPackFile));
    }
    catch (std::exception& ex)
    {
        std::cerr << "can't open archive: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    for (auto& fileInfo : ra->GetFilesInfo())
    {
        std::cout << "start unpacking: " << fileInfo.relativeFilePath
                  << " compressed: " << fileInfo.compressedSize
                  << " original: " << fileInfo.originalSize
                  << " packingType: " << ToString(fileInfo.compressionType)
                  << '\n';
        if (!UnpackFile(*ra, fileInfo))
        {
            return EXIT_FAILURE;
        }
    }
    std::cout << "Success!\n";
    return EXIT_SUCCESS;
}

int ListContent(const String& pakfile)
{
    RefPtr<FileSystem> fs(new FileSystem());
    if (!fs)
    {
        std::cerr << "can't create FileSystem\n";
        return EXIT_FAILURE;
    }
    std::unique_ptr<ResourceArchive> ra;
    try
    {
        ra.reset(new ResourceArchive(pakfile));
    }
    catch (...)
    {
        return EXIT_FAILURE;
    }

    for (auto& info : ra->GetFilesInfo())
    {
        std::cout << info.relativeFilePath << " compressed: " << info.compressedSize
                  << " original: " << info.originalSize
                  << " type: " << ToString(info.compressionType) << '\n';
    }
    return EXIT_SUCCESS;
}

static Compressor::Type GetPackingBestType(const String& fileName)
{
    return Compressor::Type::Lz4HC;
};

int main(int argc, char* argv[])
{
    ProgramOptions packOptions("pack");
    packOptions.AddOption("--compression", VariantType(String("lz4hc")),
                          "default compression method, lz4hc - default");
    packOptions.AddOption("--add_hidden", VariantType(String("false")),
                          "add hidden files to pack list (false or true)");
    // default rule pack all files into lz4hc
    packOptions.AddOption("--rule", VariantType(String(".lz4hc")),
                          "rule to select compression type like: --rule "
                          "xml.lz4 supported lz4, lz4hc, none",
                          true);
    packOptions.AddArgument("directory");
    packOptions.AddArgument("pakfile");

    ProgramOptions unpackOptions("unpack");
    unpackOptions.AddArgument("pakfile");
    unpackOptions.AddArgument("directory");

    ProgramOptions listOptions("list");
    listOptions.AddArgument("pakfile");

    if (packOptions.Parse(argc, argv))
    {
        auto dirName = packOptions.GetArgument("directory");
        auto pakFile = packOptions.GetArgument("pakfile");
        auto addHidden = packOptions.GetOption("--add_hidden").AsString();

        return PackDirectory(dirName, pakFile, addHidden == "true");
    }

    if (unpackOptions.Parse(argc, argv))
    {
        auto pakFile = unpackOptions.GetArgument("pakfile");
        auto dirName = unpackOptions.GetArgument("directory");

        return UnpackToDirectory(pakFile, dirName);
    }

    if (listOptions.Parse(argc, argv))
    {
        auto pakFile = listOptions.GetArgument("pakfile");
        return ListContent(pakFile);
    }

    {
        packOptions.PrintUsage();
        unpackOptions.PrintUsage();
        listOptions.PrintUsage();
        return EXIT_FAILURE;
    }
}

} // namespace Archiver
} // namespace DAVA
