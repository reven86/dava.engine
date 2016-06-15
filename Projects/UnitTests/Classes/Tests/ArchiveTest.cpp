#include "UnitTests/UnitTests.h"
#include <FileSystem/Private/PackArchive.h>
#include <FileSystem/Private/ZipArchive.h>
#include <FileSystem/FileSystem.h>

#include <cstring>

using namespace DAVA;

DAVA_TESTCLASS (ArchiveTest)
{
    DAVA_TEST (TestDavaArchive)
    {
        Vector<ResourceArchive::FileInfo> infos{
            ResourceArchive::FileInfo{ "Folder1/file1", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder1/file2.txt", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder1/file3.doc", 0, 0, Compressor::Type::None },

            ResourceArchive::FileInfo{ "Folder2/file1", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder2/file1.txt", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder2/file2", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder2/file2.txt", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder2/file3", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder2/file3.doc", 0, 0, Compressor::Type::None },

            ResourceArchive::FileInfo{ "Folder2/file1", 0, 0, Compressor::Type::None },
            ResourceArchive::FileInfo{ "Folder2/file3.doc", 0, 0, Compressor::Type::None },
        };

        FilePath baseDir("~res:/TestData/FileListTest/");

#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)

        {
            PackArchive archive("~res:/TestData/ArchiveTest/archive.pak");

            for (auto& info : infos)
            {
                TEST_VERIFY(archive.HasFile(info.relativeFilePath));
                const ResourceArchive::FileInfo* archiveInfo = archive.GetFileInfo(info.relativeFilePath);
                TEST_VERIFY(archiveInfo != nullptr);
                if (archiveInfo)
                {
                    TEST_VERIFY(archiveInfo->compressionType == info.compressionType);
                    TEST_VERIFY(archiveInfo->compressedSize == info.compressedSize);
                    TEST_VERIFY(archiveInfo->originalSize == info.originalSize);
                    TEST_VERIFY(archiveInfo->relativeFilePath == String(info.relativeFilePath));

                    Vector<uint8> fileContentFromArchive;

                    TEST_VERIFY(archive.LoadFile(info.relativeFilePath, fileContentFromArchive));

                    FilePath fileOnHDD = baseDir + info.relativeFilePath;

                    ScopedPtr<File> file(File::Create(fileOnHDD, File::OPEN | File::READ));
                    TEST_VERIFY(file);

                    uint32 fileSize = file->GetSize();

                    TEST_VERIFY(fileSize == fileContentFromArchive.size());

                    Vector<uint8> fileContentFromHDD(file->GetSize(), 0);
                    uint32 readBytes = file->Read(fileContentFromHDD.data(), file->GetSize());
                    TEST_VERIFY(readBytes == fileContentFromArchive.size());

                    TEST_VERIFY(fileContentFromArchive == fileContentFromHDD);
                }
            }
        }
#endif // __DAVAENGINE_IPHONE__
    }

    DAVA_TEST (TestZipArchive)
    {
        try
        {
            ZipArchive archive("~res:/TestData/ArchiveTest/archive.zip");
            {
                const char* filename = "Utf8Test/utf16le.txt";

                TEST_VERIFY(archive.HasFile(filename));

                const ResourceArchive::FileInfo* archiveInfo = archive.GetFileInfo(filename);

                TEST_VERIFY(archiveInfo->compressionType == Compressor::Type::RFC1951);

                Vector<uint8> fileFromArchive;

                TEST_VERIFY(archive.LoadFile(filename, fileFromArchive));

                FilePath filePath("~res:/TestData/Utf8Test/utf16le.txt");

                ScopedPtr<File> file(File::Create(filePath, File::OPEN | File::READ));

                uint32 fileSize = file->GetSize();

                Vector<uint8> fileFromHDD(fileSize, 0);

                file->Read(fileFromHDD.data(), fileSize);

                TEST_VERIFY(fileFromHDD == fileFromArchive);
            }
        }
        catch (std::exception& ex)
        {
            Logger::Error("%s", ex.what());
            TEST_VERIFY(false && "can't open zip file");
        }
    }
};
