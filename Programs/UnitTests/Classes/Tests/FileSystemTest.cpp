#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"
#include "Compression/LZ4Compressor.h"
#include "FileSystem/Private/PackFormatSpec.h"
#include "Utils/CRC32.h"

using namespace DAVA;

DAVA_TESTCLASS (FileSystemTest)
{
    FilePath tempDir = "~doc:/TestData/FileSystemTest_Temp/";

    FileSystemTest()
    {
        FileSystem::Instance()->DeleteDirectory("~doc:/TestData/FileSystemTest/", true);
        bool dataPrepared = FileSystem::Instance()->RecursiveCopy("~res:/TestData/FileSystemTest/", "~doc:/TestData/FileSystemTest/");
        DVASSERT(dataPrepared);
    }

    void SetUp(const String&)override
    {
        FileSystem::Instance()->CreateDirectory(tempDir);
    }

    void TearDown(const String&)override
    {
        FileSystem::Instance()->DeleteDirectory(tempDir);
    }

    DAVA_TEST (ResTestFunction)
    {
        ScopedPtr<FileList> fileList(new FileList("~res:/TestData/FileSystemTest/"));

        TEST_VERIFY(fileList->GetDirectoryCount() == 4);
        TEST_VERIFY(fileList->GetFileCount() == 0);

        for (uint32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
        {
            if (fileList->IsNavigationDirectory(ifo))
                continue;

            String filename = fileList->GetFilename(ifo);
            FilePath pathname = fileList->GetPathname(ifo);
            ScopedPtr<FileList> files(new FileList(pathname));
            TEST_VERIFY(files->GetDirectoryCount() == 0);

            if (filename == "Folder1")
            {
                TEST_VERIFY(pathname == "~res:/TestData/FileSystemTest/Folder1/");
                TEST_VERIFY(files->GetFileCount() == 3);

                for (uint32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    File* file = File::Create(pathname, File::OPEN | File::READ);
                    TEST_VERIFY(file != NULL);

                    if (!file)
                        continue;

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

    DAVA_TEST (DocTestFunctionCheckCopy)
    {
        ScopedPtr<FileList> fileList(new FileList("~doc:/TestData/FileSystemTest/"));

        TEST_VERIFY(fileList->GetDirectoryCount() == 4);
        TEST_VERIFY(fileList->GetFileCount() == 0);

        for (uint32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
        {
            if (fileList->IsNavigationDirectory(ifo))
                continue;

            String filename = fileList->GetFilename(ifo);
            FilePath pathname = fileList->GetPathname(ifo);
            ScopedPtr<FileList> files(new FileList(pathname));
            TEST_VERIFY(files->GetDirectoryCount() == 0);

            if (filename == "Folder1")
            {
                TEST_VERIFY(pathname == "~doc:/TestData/FileSystemTest/Folder1/");
                TEST_VERIFY(files->GetFileCount() == 3);

                for (uint32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    File* file = File::Create(pathname, File::OPEN | File::READ);
                    TEST_VERIFY(file != nullptr);

                    if (!file)
                        continue;

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

    DAVA_TEST (DocTestFunction)
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

        TEST_VERIFY(FilePath::FilepathInDocuments("Test/test.file")
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

#if defined(__DAVAENGINE_WINDOWS__)
        FileSystem* fs = FileSystem::Instance();
        String externalDrive = "d:\\Temp";
        bool isDdriveExist = fs->IsDirectory(externalDrive);
        if (isDdriveExist)
        {
            String testWriteFileName = externalDrive + "test_write_on_d_drive.txt";
            {
                ScopedPtr<File> testWrite(File::Create(testWriteFileName, File::OPEN | File::WRITE));
                if (testWrite)
                {
                    String tmpFileName = externalDrive + "test_on_other_drive.file";
                    moved = fs->MoveFile("~doc:/TestData/FileSystemTest/Folder1/file_new", tmpFileName, true);
                    TEST_VERIFY(moved);
                    moved = fs->MoveFile(tmpFileName, "~doc:/TestData/FileSystemTest/Folder1/file_new", true);
                    TEST_VERIFY(moved);
                }
            }
            FileSystem::Instance()->DeleteFile(testWriteFileName);
        }
#endif

        FileSystem::Instance()->DeleteFile("~doc:/TestData/FileSystemTest/Folder1/file1_new");
        File* f = File::Create("~doc:/TestData/FileSystemTest/Folder1/file1_new", File::OPEN | File::READ);
        TEST_VERIFY(!f);
        SafeRelease(f);

        uint32 count = FileSystem::Instance()->DeleteDirectoryFiles("~doc:/TestData/FileSystemTest/Folder1/");
        TEST_VERIFY(count == 2);

        ScopedPtr<FileList> fileList(new FileList("~doc:/TestData/FileSystemTest/Folder1/"));
        TEST_VERIFY(fileList->GetFileCount() == 0);
    }

    DAVA_TEST (FileOperationsTestFunction)
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

        if (!f1 || !f2)
            return;

        uint64 size = f1->GetSize();
        TEST_VERIFY(size == f2->GetSize());

        char8* buf1 = new char8[static_cast<size_t>(size)];
        char8* buf2 = new char8[static_cast<size_t>(size)];

        do
        {
            uint32 res1 = f1->ReadLine(buf1, static_cast<uint32>(size));
            uint32 res2 = f2->ReadLine(buf2, static_cast<uint32>(size));
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

        int64 seekPos = f1->GetSize() + 10;
        TEST_VERIFY(f1->Seek(seekPos, File::SEEK_FROM_START));
        TEST_VERIFY(f2->Seek(seekPos, File::SEEK_FROM_START));

        uint64 pos1 = f1->GetPos();
        uint64 pos2 = f2->GetPos();

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

    DAVA_TEST (CompareFilesTest)
    {
        String folder = "~doc:/FileSystemTest/";
        FilePath textFilePath = folder + "text";
        FilePath textFilePath2 = folder + "text2";
        FilePath binaryFilePath = folder + "binary";
        File* text = File::Create(textFilePath, File::CREATE | File::WRITE);
        File* binary = File::Create(binaryFilePath, File::CREATE | File::WRITE);

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

    DAVA_TEST (GetFrameworkPathTest)
    {
        FileSystem* fs = FileSystem::Instance();

        const FilePath tmp = fs->GetTempDirectoryPath();

        if (!tmp.IsEmpty())
        {
            String tmps = tmp.GetStringValue();

            const String dataDir = tmps + "/data/";
            if (fs->CreateDirectory(dataDir, true) != FileSystem::DIRECTORY_CANT_CREATE)
            {
                const String innerDir = tmps + "/inner_data/";
                if (fs->CreateDirectory(innerDir, true != FileSystem::DIRECTORY_CANT_CREATE))
                {
                    FilePath::AddResourcesFolder(dataDir);
                    FilePath::AddResourcesFolder(innerDir);

                    String filePath = innerDir + String("file.yaml");
                    File* f = File::Create(filePath, File::CREATE | File::WRITE);

                    if (f != nullptr)
                    {
                        SafeRelease(f);
                    }

                    String fullPath = tmps + "/inner_data/file.yaml";

                    FilePath absPath(fullPath);

                    FilePath resPath = absPath.GetFrameworkPath();
                    TEST_VERIFY(resPath.GetStringValue() == "~res:/file.yaml");

                    fs->DeleteDirectoryFiles(innerDir, true);
                    fs->DeleteDirectory(innerDir);
                }
                fs->DeleteDirectory(dataDir);
            }
        }
    }

    DAVA_TEST (IsDirectoryTest)
    {
        FileSystem* fs = FileSystem::Instance();

        FilePath dirPath = tempDir + "Dir/";

        TEST_VERIFY(fs->IsDirectory(dirPath) == false);
        TEST_VERIFY(fs->IsFile(dirPath) == false);

        FileSystem::eCreateDirectoryResult res = fs->CreateDirectory(dirPath);
        TEST_VERIFY(res == FileSystem::eCreateDirectoryResult::DIRECTORY_CREATED)
        TEST_VERIFY(fs->IsDirectory(dirPath) == true);
        TEST_VERIFY(fs->IsFile(dirPath) == false);
    }

    DAVA_TEST (IsFileTest)
    {
        FileSystem* fs = FileSystem::Instance();

        FilePath filePath = tempDir + "file";

        TEST_VERIFY(fs->IsDirectory(filePath) == false);
        TEST_VERIFY(fs->IsFile(filePath) == false);

        ScopedPtr<File> file(File::Create(filePath, File::CREATE | File::WRITE));
        TEST_VERIFY(file.get() != nullptr)
        TEST_VERIFY(fs->IsDirectory(filePath) == false);
        TEST_VERIFY(fs->IsFile(filePath) == true);
    }

    DAVA_TEST (CreateFilePassingDirTest)
    {
        FileSystem* fs = FileSystem::Instance();

        FilePath dirPath = tempDir + "Dir/";

        ScopedPtr<File> file(File::Create(dirPath, File::CREATE | File::WRITE));
        TEST_VERIFY(file.get() == nullptr)
    }

    DAVA_TEST (CreateFilePassingExistingDirTest)
    {
        FileSystem* fs = FileSystem::Instance();

        FilePath dirPath = tempDir + "Dir/";
        fs->CreateDirectory(dirPath);

        ScopedPtr<File> file(File::Create(dirPath, File::CREATE | File::WRITE));
        TEST_VERIFY(file.get() == nullptr)
    }

    DAVA_TEST (LoadCompressedMiniPackFile)
    {
        FileSystem* fs = FileSystem::Instance();

        // first: generate compressed file
        {
            Vector<uint8> buff(1024, 'A');
            Vector<uint8> compressedBuff;
            LZ4HCCompressor().Compress(buff, compressedBuff);

            FilePath filePath = tempDir + "file.txt.dvpl";
            fs->DeleteFile(filePath);

            ScopedPtr<File> file(File::Create(filePath, File::CREATE | File::WRITE));

            file->Write(&compressedBuff[0], static_cast<uint32>(compressedBuff.size()));

            PackFormat::MiniPack::Footer footer;
            footer.sizeCompressed = static_cast<uint32>(compressedBuff.size());
            footer.sizeUncompressed = static_cast<uint32>(buff.size());
            footer.type = Compressor::Type::Lz4HC;
            footer.crc32Compressed = CRC32::ForBuffer(&compressedBuff[0], compressedBuff.size());
            footer.packMarkerLite = PackFormat::FILE_MARKER_LITE;

            file->Write(&footer, sizeof(footer));
        }

        // second: load just generated file and check
        {
            FilePath filePath = tempDir + "file.txt";
            TEST_VERIFY(fs->IsFile(filePath));

            ScopedPtr<File> file(File::Create(filePath, File::OPEN | File::READ));
            if (!file)
            {
                TEST_VERIFY(false)
            }
            else
            {
                TEST_VERIFY(file->GetSize() == 1024);
                String content = fs->ReadFileContents(filePath);
                TEST_VERIFY(content == String(1024, 'A'));
            }
            // clear
            TEST_VERIFY(fs->DeleteFile(filePath + ".dvpl"))
        }
    }
}
;
