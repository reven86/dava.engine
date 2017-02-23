#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)

#include "Tools/ResourceArchiver/ResourceArchiver.h"

using namespace DAVA;

DAVA_TESTCLASS (ResourceArchiverTest)
{
    ResourceArchiverTest()
    {
        FileSystem::Instance()->DeleteDirectory("~doc:/TestData/ResourceArchiverTest/", true);
        FileSystem::Instance()->RecursiveCopy("~res:/TestData/ResourceArchiverTest/", "~doc:/TestData/ResourceArchiverTest/");
        FileSystem::Instance()->CreateDirectory("~doc:/TestData/ResourceArchiverTest/SampleSource/Recursive/AccountScreen/EmptyFolder/");
    }

    DAVA_TEST (TestCreateWithCompression)
    {
        ResourceArchiver::Params params;
        params.sourcesList = { "~doc:/TestData/ResourceArchiverTest/SampleSource/" };
        params.archivePath = "~doc:/TestData/ResourceArchiverTest/result.archive";
        params.baseDirPath = FileSystem::Instance()->GetCurrentWorkingDirectory();
        SCOPE_EXIT
        {
            FileSystem::Instance()->DeleteFile(params.archivePath);
        };

        params.compressionType = Compressor::Type::Lz4;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));

        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4.archive"));
        VerifyCompression(params.archivePath, Compressor::Type::Lz4);

        params.compressionType = Compressor::Type::Lz4HC;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));
        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4hc.archive"));
        VerifyCompression(params.archivePath, Compressor::Type::Lz4HC);

        params.compressionType = Compressor::Type::RFC1951;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));
        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_rfc1951.archive"));
        VerifyCompression(params.archivePath, Compressor::Type::RFC1951);

        params.compressionType = Compressor::Type::None;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));
        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_nocompression.archive"));
        VerifyCompression(params.archivePath, Compressor::Type::None);
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
        catch (std::exception& ex)
        {
            TEST_VERIFY_WITH_MESSAGE(false, Format("Can't open archive %s: %s", archivePath.GetAbsolutePathname().c_str(), ex.what()));
        }
    }

    DAVA_TEST (TestCreateWithHidden)
    {
        ResourceArchiver::Params params;
        params.sourcesList = { "~doc:/TestData/ResourceArchiverTest/SampleSource/" };
        params.archivePath = "~doc:/TestData/ResourceArchiverTest/result.archive";
        params.baseDirPath = FileSystem::Instance()->GetCurrentWorkingDirectory();
        SCOPE_EXIT
        {
            FileSystem::Instance()->DeleteFile(params.archivePath);
        };
        params.compressionType = Compressor::Type::Lz4;

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

        params.addHiddenFiles = true;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));
        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4.archive"));
        TEST_VERIFY(true == IsFileExistsInArchive(params.archivePath, "SampleSource/Recursive/flags.txt"));

        params.addHiddenFiles = false;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));
        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4_nohidden.archive"));
        TEST_VERIFY(false == IsFileExistsInArchive(params.archivePath, "SampleSource/Recursive/flags.txt"));

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        FilePath file1 = "~doc:/TestData/ResourceArchiverTest/SampleSource/Recursive/flags.txt";
        FilePath file1hidden = "~doc:/TestData/ResourceArchiverTest/SampleSource/Recursive/.flags.txt";
        TEST_VERIFY(true == FileSystem::Instance()->CopyFile(file1, file1hidden, true));
        SCOPE_EXIT
        {
            FileSystem::Instance()->DeleteFile(file1hidden);
        };

        params.addHiddenFiles = true;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));
        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4_withhidden.archive"));
        TEST_VERIFY(true == IsFileExistsInArchive(params.archivePath, "SampleSource/Recursive/.flags.txt"));

        params.addHiddenFiles = false;
        TEST_VERIFY(true == ResourceArchiver::CreateArchive(params));
        //TEST_VERIFY(true == FileSystem::Instance()->CompareBinaryFiles(params.archivePath, "~res:/TestData/ResourceArchiverTest/SampleArchives/result_lz4.archive"));
        TEST_VERIFY(false == IsFileExistsInArchive(params.archivePath, "SampleSource/Recursive/.flags.txt"));

#endif //PLATFORMS
    }

    bool IsFileExistsInArchive(const FilePath& archivePath, const String& pathInArchive)
    {
        try
        {
            ResourceArchive resourceArchive(archivePath);
            return resourceArchive.HasFile(pathInArchive);
        }
        catch (std::exception& ex)
        {
            TEST_VERIFY_WITH_MESSAGE(false, Format("Can't open archive %s: %s", archivePath.GetAbsolutePathname().c_str(), ex.what()));
        }

        return false;
    }
};

#endif
