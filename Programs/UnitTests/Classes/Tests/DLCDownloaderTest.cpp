#include "UnitTests/UnitTests.h"

#include "DLCManager/Private/DLCDownloaderImpl.h"

DAVA_TESTCLASS (DLCDownloaderTest)
{
    DAVA_TEST (GetFileSizeTest)
    {
        using namespace DAVA;

        DLCDownloaderImpl downloader;

        downloader.StartTask("", nullptr, DLCDownloader::TaskType::SIZE);
    }
};