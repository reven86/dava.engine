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
#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS(FileSystemTest)
{
    DEDUCE_COVERED_CLASS_FROM_TESTCLASS()

    FileSystemTest()
    {
        FileSystem::Instance()->DeleteDirectory("~doc:/TestData/FileSystemTest/", true);
        bool dataPrepared = RecursiveCopy("~res:/TestData/FileSystemTest/", "~doc:/TestData/FileSystemTest/");
        DVASSERT(dataPrepared);
    }

    DAVA_TEST(ResTestFunction)
    {
        ScopedPtr<FileList> fileList(new FileList("~res:/TestData/FileSystemTest/"));

        TEST_VERIFY(fileList->GetDirectoryCount() == 4);
        TEST_VERIFY(fileList->GetFileCount() == 0);

        for (int32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
        {
            if (fileList->IsNavigationDirectory(ifo)) continue;

            String filename = fileList->GetFilename(ifo);
            FilePath pathname = fileList->GetPathname(ifo);
            ScopedPtr<FileList> files(new FileList(pathname));
            TEST_VERIFY(files->GetDirectoryCount() == 0);

            if (filename == "Folder1")
            {
                TEST_VERIFY(pathname == "~res:/TestData/FileSystemTest/Folder1/");
                TEST_VERIFY(files->GetFileCount() == 3);

                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi)) continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    File *file = File::Create(pathname, File::OPEN | File::READ);
                    TEST_VERIFY(file != NULL);

                    if (!file) continue;

                    if (filename == "file1.zip")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileSystemTest/Folder1/file1.zip");
                        TEST_VERIFY(file->GetFilename() == "~res:/TestData/FileSystemTest/Folder1/file1.zip");
                        TEST_VERIFY(file->GetSize() == 31749);
                    }
                    else if (filename == "file2.zip")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileSystemTest/Folder1/file2.zip");
                        TEST_VERIFY(file->GetFilename() == "~res:/TestData/FileSystemTest/Folder1/file2.zip");
                        TEST_VERIFY(file->GetSize() == 22388);
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileSystemTest/Folder1/file3.doc");
                        TEST_VERIFY(file->GetFilename() == "~res:/TestData/FileSystemTest/Folder1/file3.doc");
                        TEST_VERIFY(file->GetSize() == 37479);
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }

                    SafeRelease(file);
                }
            }
        }
    }

    DAVA_TEST(DocTestFunctionCheckCopy)
    {
        ScopedPtr<FileList> fileList(new FileList("~doc:/TestData/FileSystemTest/"));

        TEST_VERIFY(fileList->GetDirectoryCount() == 4);
        TEST_VERIFY(fileList->GetFileCount() == 0);

        for (int32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
        {
            if (fileList->IsNavigationDirectory(ifo)) continue;

            String filename = fileList->GetFilename(ifo);
            FilePath pathname = fileList->GetPathname(ifo);
            ScopedPtr<FileList> files(new FileList(pathname));
            TEST_VERIFY(files->GetDirectoryCount() == 0);

            if (filename == "Folder1")
            {
                TEST_VERIFY(pathname == "~doc:/TestData/FileSystemTest/Folder1/");
                TEST_VERIFY(files->GetFileCount() == 3);

                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi)) continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    File *file = File::Create(pathname, File::OPEN | File::READ);
                    TEST_VERIFY(file != nullptr);

                    if (!file) continue;

                    if (filename == "file1.zip")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileSystemTest/Folder1/file1.zip");
                        TEST_VERIFY(file->GetFilename() == "~doc:/TestData/FileSystemTest/Folder1/file1.zip");
                        TEST_VERIFY(file->GetSize() == 31749);
                    }
                    else if (filename == "file2.zip")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileSystemTest/Folder1/file2.zip");
                        TEST_VERIFY(file->GetFilename() == "~doc:/TestData/FileSystemTest/Folder1/file2.zip");
                        TEST_VERIFY(file->GetSize() == 22388);
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileSystemTest/Folder1/file3.doc");
                        TEST_VERIFY(file->GetFilename() == "~doc:/TestData/FileSystemTest/Folder1/file3.doc");
                        TEST_VERIFY(file->GetSize() == 37479);
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }

                    SafeRelease(file);
                }
            }
        }
    }

    DAVA_TEST(DocTestFunction)
    {
        FilePath savedCurrentWorkingDirectory = FileSystem::Instance()->GetCurrentWorkingDirectory();
        TEST_VERIFY(FileSystem::Instance()->SetCurrentWorkingDirectory("~doc:/TestData/FileSystemTest/"));
        TEST_VERIFY(FileSystem::Instance()->SetCurrentWorkingDirectory(savedCurrentWorkingDirectory));

        FilePath savedCurrentDocDirectory = FileSystem::Instance()->GetCurrentDocumentsDirectory();
        FileSystem::Instance()->SetCurrentDocumentsDirectory("~doc:/TestData/FileSystemTest/");
        TEST_VERIFY(FileSystem::Instance()->GetCurrentDocumentsDirectory() == "~doc:/");

        FileSystem::Instance()->SetCurrentDocumentsDirectory(savedCurrentDocDirectory);
        TEST_VERIFY(FileSystem::Instance()->GetCurrentDocumentsDirectory() == savedCurrentDocDirectory);


        TEST_VERIFY(!FileSystem::Instance()->IsDirectory("~doc:/TestData/FileSystemTest/Folder1/file1.zip"));
        TEST_VERIFY(FileSystem::Instance()->IsFile("~doc:/TestData/FileSystemTest/Folder1/file1.zip"));

        TEST_VERIFY(FileSystem::Instance()->IsDirectory("~doc:/TestData/FileSystemTest/"));
        TEST_VERIFY(!FileSystem::Instance()->IsFile("~doc:/TestData/FileSystemTest/"));


        TEST_VERIFY(FilePath::FilepathInDocuments((const char *)"Test/test.file")
                    == FilePath::FilepathInDocuments(String("Test/test.file")));


        FileSystem::eCreateDirectoryResult created = FileSystem::Instance()->CreateDirectory("~doc:/TestData/FileSystemTest/1/2/3", false);
        TEST_VERIFY(created == FileSystem::DIRECTORY_CANT_CREATE);
        created = FileSystem::Instance()->CreateDirectory("~doc:/TestData/FileSystemTest/1/2/3", true);
        TEST_VERIFY(created == FileSystem::DIRECTORY_CREATED);
        created = FileSystem::Instance()->CreateDirectory("~doc:/TestData/FileSystemTest/1/2/3", false);
        TEST_VERIFY(created == FileSystem::DIRECTORY_EXISTS);



        bool moved = FileSystem::Instance()->MoveFile("~doc:/TestData/FileSystemTest/Folder1/file1.zip", "~doc:/TestData/FileSystemTest/Folder1/file_new");
        TEST_VERIFY(moved);

        moved = FileSystem::Instance()->MoveFile("~doc:/TestData/FileSystemTest/Folder1/file2.zip", "~doc:/TestData/FileSystemTest/Folder1/file_new");
        TEST_VERIFY(!moved);

        moved = FileSystem::Instance()->MoveFile("~doc:/TestData/FileSystemTest/Folder1/file2.zip", "~doc:/TestData/FileSystemTest/Folder1/file_new", true);
        TEST_VERIFY(moved);

#if   defined(__DAVAENGINE_WINDOWS__)
        FileSystem* fs = FileSystem::Instance();
        String externalDrive = "d:\\";
        bool isDdriveExist = fs->IsDirectory(externalDrive);
        if (isDdriveExist)
        {
            String tmpFileName = externalDrive + "test_on_other_drive.file";
            moved = fs->MoveFile("~doc:/TestData/FileSystemTest/Folder1/file_new", tmpFileName, true);
            TEST_VERIFY(moved);
            moved = fs->MoveFile(tmpFileName, "~doc:/TestData/FileSystemTest/Folder1/file_new", true);
            TEST_VERIFY(moved);
        }
#endif

        FileSystem::Instance()->DeleteFile("~doc:/TestData/FileSystemTest/Folder1/file1_new");
        File *f = File::Create("~doc:/TestData/FileSystemTest/Folder1/file1_new", File::OPEN | File::READ);
        TEST_VERIFY(!f);
        SafeRelease(f);


        uint32 count = FileSystem::Instance()->DeleteDirectoryFiles("~doc:/TestData/FileSystemTest/Folder1/");
        TEST_VERIFY(count == 2);

        ScopedPtr<FileList> fileList(new FileList("~doc:/TestData/FileSystemTest/Folder1/"));
        TEST_VERIFY(fileList->GetFileCount() == 0);
    }

    DAVA_TEST(FileOperationsTestFunction)
    {
        FilePath fileInAssets = "~res:/TestData/FileSystemTest/FileTest/test.yaml";
        FilePath cpyDir = "~doc:/FileSystemTest/FileTest/";
        FilePath copyTo = cpyDir + "test.yaml";

        FileSystem::Instance()->CreateDirectory(cpyDir, true);

        TEST_VERIFY(FileSystem::Instance()->CopyFile(fileInAssets, copyTo, true));

        File* f1 = File::Create(fileInAssets, File::OPEN | File::READ);
        File* f2 = File::Create(copyTo, File::OPEN | File::READ);

        TEST_VERIFY(NULL != f1);
        TEST_VERIFY(NULL != f2);

        if (!f2 || !f2)
            return;

        uint32 size = f1->GetSize();
        TEST_VERIFY(size == f2->GetSize());

        char8 *buf1 = new char8[size];
        char8 *buf2 = new char8[size];

        do
        {
            uint32 res1 = f1->ReadLine(buf1, size);
            uint32 res2 = f2->ReadLine(buf2, size);
            TEST_VERIFY(res1 == res2);
            TEST_VERIFY(!Memcmp(buf1, buf2, res1));

        } while (!f1->IsEof());

        TEST_VERIFY(f2->IsEof());

        SafeRelease(f1);
        SafeRelease(f2);

        SafeDeleteArray(buf1);
        SafeDeleteArray(buf2);

        f1 = File::Create(fileInAssets, File::OPEN | File::READ);
        f2 = File::Create(copyTo, File::OPEN | File::READ);

        int32 seekPos = f1->GetSize() + 10;
        TEST_VERIFY(f1->Seek(seekPos, File::SEEK_FROM_START));
        TEST_VERIFY(f2->Seek(seekPos, File::SEEK_FROM_START));

        uint32 pos1 = f1->GetPos();
        uint32 pos2 = f2->GetPos();

        TEST_VERIFY(f1->IsEof() == false);
        TEST_VERIFY(f2->IsEof() == false);

        TEST_VERIFY(pos1 == seekPos);
        TEST_VERIFY(pos2 == seekPos);

        seekPos = (seekPos + 20); // seek to -20 pos
        f1->Seek(-seekPos, File::SEEK_FROM_CURRENT);
        f2->Seek(-seekPos, File::SEEK_FROM_CURRENT);

        pos1 = f1->GetPos();
        pos2 = f2->GetPos();

        TEST_VERIFY(f1->IsEof() == f2->IsEof());
        TEST_VERIFY(pos1 == pos2);

        SafeRelease(f1);
        SafeRelease(f2);

        FileSystem::Instance()->DeleteFile(copyTo);
    }

    DAVA_TEST(CompareFilesTest)
    {
        String folder = "~doc:/FileSystemTest/";
        FilePath textFilePath = folder + "text";
        FilePath textFilePath2 = folder + "text2";
        FilePath binaryFilePath = folder + "binary";
        File *text = File::Create(textFilePath, File::CREATE | File::WRITE);
        File *binary = File::Create(binaryFilePath, File::CREATE | File::WRITE);

        text->WriteLine("1");
        binary->Write("1");
        SafeRelease(text);
        SafeRelease(binary);
        FilePath cpyFilePath = folder + "cpy";
        FileSystem::Instance()->CopyFile(textFilePath, cpyFilePath);
        TEST_VERIFY(FileSystem::Instance()->CompareTextFiles(textFilePath, cpyFilePath));
        TEST_VERIFY(FileSystem::Instance()->CompareBinaryFiles(textFilePath, cpyFilePath));
        TEST_VERIFY(!FileSystem::Instance()->CompareTextFiles(textFilePath, binaryFilePath));
        TEST_VERIFY(!FileSystem::Instance()->CompareBinaryFiles(textFilePath, binaryFilePath));
        FileSystem::Instance()->DeleteFile(textFilePath);
        FileSystem::Instance()->DeleteFile(cpyFilePath);
        FileSystem::Instance()->DeleteFile(binaryFilePath);

        text = File::Create(textFilePath, File::CREATE | File::WRITE);
        text->WriteLine("");
        text->WriteLine("");
        text->WriteLine("1");
        SafeRelease(text);
        text = File::Create(textFilePath2, File::CREATE | File::WRITE);
        text->WriteLine("1");
        SafeRelease(text);
        TEST_VERIFY(!FileSystem::Instance()->CompareTextFiles(textFilePath, textFilePath2));
        TEST_VERIFY(!FileSystem::Instance()->CompareBinaryFiles(textFilePath, textFilePath2));
        FileSystem::Instance()->DeleteFile(textFilePath);
        FileSystem::Instance()->DeleteFile(textFilePath2);

        text = File::Create(textFilePath, File::CREATE | File::WRITE);
        text->WriteLine("");
        text->WriteLine("1");
        text->WriteLine("");
        SafeRelease(text);
        text = File::Create(textFilePath2, File::CREATE | File::WRITE);
        text->WriteLine("1");
        SafeRelease(text);
        TEST_VERIFY(!FileSystem::Instance()->CompareTextFiles(textFilePath, textFilePath2));
        TEST_VERIFY(!FileSystem::Instance()->CompareBinaryFiles(textFilePath, textFilePath2));
        FileSystem::Instance()->DeleteFile(textFilePath);
        FileSystem::Instance()->DeleteFile(textFilePath2);

        text = File::Create(textFilePath, File::CREATE | File::WRITE);
        text->WriteLine("1");
        SafeRelease(text);
        text = File::Create(textFilePath2, File::CREATE | File::WRITE);
        text->WriteLine("1");
        SafeRelease(text);
        TEST_VERIFY(FileSystem::Instance()->CompareTextFiles(textFilePath, textFilePath2));
        TEST_VERIFY(FileSystem::Instance()->CompareBinaryFiles(textFilePath, textFilePath2));
        FileSystem::Instance()->DeleteFile(textFilePath);
        FileSystem::Instance()->DeleteFile(textFilePath2);

        text = File::Create(textFilePath, File::CREATE | File::WRITE);
        text->Write("1");
        text->Write("\r");
        text->Write("\n");
        SafeRelease(text);
        text = File::Create(textFilePath2, File::CREATE | File::WRITE);
        text->Write("1");
        text->Write("\n");
        SafeRelease(text);
        TEST_VERIFY(FileSystem::Instance()->CompareTextFiles(textFilePath, textFilePath2));
        TEST_VERIFY(!FileSystem::Instance()->CompareBinaryFiles(textFilePath, textFilePath2));
        FileSystem::Instance()->DeleteFile(textFilePath);
        FileSystem::Instance()->DeleteFile(textFilePath2);
    }

    bool RecursiveCopy(const DAVA::FilePath &src, const DAVA::FilePath &dst)
    {
        DVASSERT(src.IsDirectoryPathname() && dst.IsDirectoryPathname());

        FileSystem::eCreateDirectoryResult created = FileSystem::Instance()->CreateDirectory(dst, true);
        DVASSERT(created != FileSystem::DIRECTORY_CANT_CREATE);

        bool copied = FileSystem::Instance()->CopyDirectory(src, dst);
        DVASSERT(copied);

        bool retCode = true;
        ScopedPtr<FileList> fileList(new FileList(src));
        for (int32 i = 0; i < fileList->GetCount(); ++i)
        {
            if (fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
            {
                retCode &= RecursiveCopy(fileList->GetPathname(i), dst + (fileList->GetFilename(i) + "/"));
            }
        }
        return retCode;
    }
};
