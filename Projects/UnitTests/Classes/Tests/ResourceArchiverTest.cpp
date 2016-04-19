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
    //DEDUCE_COVERED_CLASS_FROM_TESTCLASS()

    //     ResourceArchiverTest()
    //     {
    //         FileSystem::Instance()->DeleteDirectory("~doc:/TestData/ResourceArchiverTest/", true);
    //         RecursiveCopy("~res:/TestData/ResourceArchiverTest/", "~doc:/TestData/ResourceArchiverTest/");
    //
    //     #if defined(__DAVAENGINE_WINDOWS__)
    //         FileSystem::Instance()->DeleteDirectory("~doc:/TestData/ResourceArchiverTestWindowsExtension/", true);
    //         RecursiveCopy("~res:/TestData/ResourceArchiverTestWindowsExtension/", "~doc:/TestData/ResourceArchiverTestWindowsExtension/");
    //     #endif
    //     }

    DAVA_TEST (TestCreateWithCompression)
    {
        UniquePtr<FileList> fileList(new FileList("~res:/TestData/ResourceArchiverTest/SampleSource/"));
    }

    void TestCreateArchive(Compressor::Type compression)
    {
        ResourceArchiver::CreateArchive(sources, false, compression, "result.archive", "result.log", nullptr);
    }

    DAVA_TEST (HiddenFileTest)
    {
#if defined(__DAVAENGINE_WIN32__)
        FilePath file1 = FilePath("~doc:/TestData/ResourceArchiverTest/Folder1/file1");
        auto file1str = file1.GetAbsolutePathname();
        auto attrs = GetFileAttributesA(file1str.c_str());

        if (attrs & FILE_ATTRIBUTE_HIDDEN)
        {
            SetFileAttributesA(file1str.c_str(), attrs ^ FILE_ATTRIBUTE_HIDDEN);
        }

        ScopedPtr<FileList> files(new FileList("~doc:/TestData/ResourceArchiverTest/Folder1/"));
        TEST_VERIFY(files->GetFileCount() == 3);
        auto i = GetIndex(files, "file1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == false);

        SetFileAttributesA(file1str.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);

        files = new FileList("~doc:/TestData/ResourceArchiverTest/Folder1/");
        TEST_VERIFY(files->GetFileCount() == 3);
        i = GetIndex(files, "file1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == true);

        SetFileAttributesA(file1str.c_str(), attrs);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        FilePath file1 = "~doc:/TestData/ResourceArchiverTest/Folder1/file1";
        FilePath file1hidden = "~doc:/TestData/ResourceArchiverTest/Folder1/.file1";
        TEST_VERIFY(FileSystem::Instance()->CopyFile(file1, file1hidden, true));
        ScopedPtr<FileList> files(new FileList("~doc:/TestData/ResourceArchiverTest/Folder1/"));
        TEST_VERIFY(files->GetFileCount() == 4);
        for (auto i = 0; i < files->GetCount(); ++i)
        {
            if (!files->IsDirectory(i))
            {
                bool startsWithDot = (files->GetFilename(i)[0] == '.');
                TEST_VERIFY(files->IsHidden(i) == startsWithDot);
            }
        }
        FileSystem::Instance()->DeleteFile(file1hidden);
#endif //PLATFORMS
    }

    DAVA_TEST (HiddenDirTest)
    {
#if defined(__DAVAENGINE_WIN32__)
        FilePath dir1 = FilePath("~doc:/TestData/ResourceArchiverTest/Folder1/");
        auto dir1str = dir1.GetAbsolutePathname();
        auto attrs = GetFileAttributesA(dir1str.c_str());

        if (attrs & FILE_ATTRIBUTE_HIDDEN)
        {
            SetFileAttributesA(dir1str.c_str(), attrs ^ FILE_ATTRIBUTE_HIDDEN);
        }

        ScopedPtr<FileList> files(new FileList("~doc:/TestData/ResourceArchiverTest/"));
        TEST_VERIFY(files->GetDirectoryCount() == 3);
        auto i = GetIndex(files, "Folder1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == false);

        SetFileAttributesA(dir1str.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);

        files = new FileList("~doc:/TestData/ResourceArchiverTest/");
        TEST_VERIFY(files->GetDirectoryCount() == 3);
        i = GetIndex(files, "Folder1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == true);

        SetFileAttributesA(dir1str.c_str(), attrs);

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

        FilePath folder1hidden = "~doc:/TestData/ResourceArchiverTest/.Folder1/";
        TEST_VERIFY(FileSystem::Instance()->CreateDirectory(folder1hidden, true));
        ScopedPtr<FileList> files(new FileList("~doc:/TestData/ResourceArchiverTest/"));
        TEST_VERIFY(files->GetDirectoryCount() == 4);
        for (auto i = 0; i < files->GetCount(); ++i)
        {
            if (files->IsDirectory(i))
            {
                bool startsWithDot = (files->GetFilename(i)[0] == '.');
                TEST_VERIFY(files->IsHidden(i) == startsWithDot);
            }
        }
        FileSystem::Instance()->DeleteDirectory(folder1hidden);

#endif //PLATFORMS
    }
};
