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

#include "UnitTests/UnitTests.h"
#include <FileSystem/DavaArchive.h>
#include <FileSystem/ZipArchive.h>
#include <FileSystem/FileSystem.h>

using namespace DAVA;

DAVA_TESTCLASS (ArchiveTest)
{
    DAVA_TEST (TestDavaArchive)
    {
        Vector<ResourceArchive::FileInfo> infos{
            { "TestData/Utf8Test/utf16le.txt", 0, 0, Compressor::Type::Lz4HC },
            { "TestData/LoadImageTest/10x10_rgba8888_norle.tga", 0, 0, Compressor::Type::Lz4HC },
        };

        FilePath baseDir("~res:/");

#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)

        TEST_VERIFY(DavaArchive::Create("dava.pak", baseDir, infos, nullptr));

        {
            DavaArchive archive("dava.pak");

            for (auto& info : infos)
            {
                TEST_VERIFY(archive.HasFile(info.fileName));
                const ResourceArchive::FileInfo* archiveInfo = archive.GetFileInfo(info.fileName);
                TEST_VERIFY(archiveInfo != nullptr);
                if (archiveInfo)
                {
                    TEST_VERIFY(archiveInfo->compressionType == info.compressionType);
                    TEST_VERIFY(archiveInfo->compressedSize != 0);
                    TEST_VERIFY(archiveInfo->originalSize != 0);
                    TEST_VERIFY(archiveInfo->compressedSize != archiveInfo->originalSize);

                    Vector<uint8> fileContentFromArchive;

                    TEST_VERIFY(archive.LoadFile(info.fileName, fileContentFromArchive));

                    String fileOnHDD = baseDir.GetAbsolutePathname() + info.fileName;

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

        TEST_VERIFY(FileSystem::Instance()->DeleteFile("dava.pak"));
#endif // __DAVAENGINE_IPHONE__
    }

    DAVA_TEST (TestZipArchive)
    {
        ZipArchive archive("~res:/TestData/ArchiveTest/archive.zip");

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
};
