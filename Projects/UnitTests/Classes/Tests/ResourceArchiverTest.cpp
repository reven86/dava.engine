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

#include "DAVAEngine.h"
#include "Base/UniquePtr.h"
#include "UnitTests/UnitTests.h"

#include "ResourceArchiver/ResourceArchiver.h"

using namespace DAVA;

DAVA_TESTCLASS (ResourceArchiverTest)
{
    ResourceArchiverTest()
    {
        FileSystem::Instance()->DeleteDirectory("~doc:/TestData/ResourceArchiverTest/", true);
        FileSystem::Instance()->RecursiveCopy("~res:/TestData/ResourceArchiverTest/", "~doc:/TestData/ResourceArchiverTest/");
    }

    DAVA_TEST (TestCreateWithCompression)
    {
        Vector<String> sampleSource = { "~res:/TestData/ResourceArchiverTest/SampleSource/" };
        FilePath archivePath("result.archive");
        SCOPE_EXIT
        {
            FileSystem::Instance()->DeleteFile(archivePath);
        };

        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, false, Compressor::Type::Lz4, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4.archive"));
        VerifyCompression(archivePath, Compressor::Type::Lz4);

        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, false, Compressor::Type::Lz4HC, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4hc.archive"));
        VerifyCompression(archivePath, Compressor::Type::Lz4HC);

        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, false, Compressor::Type::RFC1951, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_rfc1951.archive"));
        VerifyCompression(archivePath, Compressor::Type::RFC1951);

        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, false, Compressor::Type::None, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_nocompression.archive"));
        VerifyCompression(archivePath, Compressor::Type::None);
    }

    void VerifyCompression(const FilePath& archivePath, Compressor::Type compression)
    {
        try
        {
            ResourceArchive resourceArchive(archivePath);

            for (const ResourceArchive::FileInfo& fileInfo : resourceArchive.GetFilesInfo())
            {
                TEST_VERIFY(fileInfo.compressionType == compression || fileInfo.compressionType == Compressor::Type::None);
            }
        }
        catch (std::exception ex)
        {
            TEST_VERIFY_WITH_MESSAGE(false, Format("Can't open archive %s: %s", archivePath.GetAbsolutePathname().c_str(), ex.what()));
        }
    }

    DAVA_TEST (TestCreateWithHidden)
    {
#if defined(__DAVAENGINE_WIN32__)
        FilePath file1 = FilePath("~doc:/TestData/ResourceArchiverTest/SampleSource/Recursive/flags.txt");
        auto file1str = file1.GetAbsolutePathname();

        auto attrs = GetFileAttributesA(file1str.c_str());
        SCOPE_EXIT
        {
            SetFileAttributesA(file1str.c_str(), attrs);
        };

        if (!(attrs & FILE_ATTRIBUTE_HIDDEN))
        {
            SetFileAttributesA(file1str.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
        }

        Vector<String> sampleSource = { "~doc:/TestData/ResourceArchiverTest/SampleSource/" };
        FilePath archivePath("result.archive");
        SCOPE_EXIT
        {
            FileSystem::Instance()->DeleteFile(archivePath);
        };

        bool useHidden = true;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, useHidden, Compressor::Type::Lz4, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4.archive"));
        TEST_VERIFY(true == IsFileExistsInArchive(archivePath, "SampleSource/Recursive/flags.txt"));

        useHidden = false;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, useHidden, Compressor::Type::Lz4, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4_nohidden.archive"));
        TEST_VERIFY(false == IsFileExistsInArchive(archivePath, "SampleSource/Recursive/flags.txt"));

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        FilePath file1 = "~doc:/TestData/ResourceArchiverTest/SampleSource/Recursive/flags.txt";
        FilePath file1hidden = "~doc:/TestData/ResourceArchiverTest/SampleSource/Recursive/.flags.txt";
        TEST_VERIFY(true == FileSystem::Instance()->CopyFile(file1, file1hidden, true));
        SCOPE_EXIT
        {
            FileSystem::Instance()->DeleteFile(file1hidden);
        };

        bool useHidden = true;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, useHidden, Compressor::Type::Lz4, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4_withhidden.archive"));
        TEST_VERIFY(true == IsFileExistsInArchive(archivePath, "SampleSource/Recursive/.flags.txt"));

        useHidden = false;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(sampleSource, useHidden, Compressor::Type::Lz4, archivePath, "", nullptr));
        TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4.archive"));
        TEST_VERIFY(false == IsFileExistsInArchive(archivePath, "SampleSource/Recursive/.flags.txt"));

#endif //PLATFORMS
    }

    bool IsFileExistsInArchive(const FilePath& archivePath, const String& pathInArchive)
    {
        try
        {
            ResourceArchive resourceArchive(archivePath);

            for (const ResourceArchive::FileInfo& fileInfo : resourceArchive.GetFilesInfo())
            {
                if (fileInfo.relativeFilePath == pathInArchive)
                {
                    return true;
                }
            }
        }
        catch (std::exception ex)
        {
            TEST_VERIFY_WITH_MESSAGE(false, Format("Can't open archive %s: %s", archivePath.GetAbsolutePathname().c_str(), ex.what()));
        }

        return false;
    }
};
