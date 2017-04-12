#include "UnitTests/UnitTests.h"

#include "DLCManager/Private/DLCDownloaderImpl.h"

#include <Logger/Logger.h>
#include <Time/SystemTimer.h>
#include <DLC/Downloader/DownloadManager.h>

DAVA_TESTCLASS (DLCDownloaderTest)
{
    DAVA_TEST (GetFileSizeTest)
    {
        using namespace DAVA;

        DLCDownloaderImpl downloader;
        String url = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.245.dvpk";
        DLCDownloader::Task* task = downloader.StartTask(url, nullptr, DLCDownloader::TaskType::SIZE);

        downloader.WaitTask(task);

        TEST_VERIFY(task->status.state == DLCDownloader::TaskState::Finished);
        TEST_VERIFY(task->status.sizeTotal >= 0);
    }

    DAVA_TEST (DowloadLargeFileTest)
    {
        using namespace DAVA;

        DLCDownloaderImpl downloader;
        String url = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.245.dvpk";
        FilePath path("~doc:/big_tmp_file_from_server.remove.me");
        String p = path.GetAbsolutePathname();
        int64 start = 0;
        DLCDownloader::Task* task = nullptr;
        int64 finish = 0;
        float seconds = 0.f;

        DownloadManager* dm = DownloadManager::Instance();

        FilePath pathOld("~doc:/big_tmp_file_from_server.old.remove.me");

        //----first--------------------------------------------------------
        start = SystemTimer::GetMs();

        uint32 id = dm->Download(url, pathOld);

        dm->Wait(id);

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("old downloader 1.5 Gb download from in house server for: %f", seconds);
        // ----next-------------------------------------------------------
        {
            start = SystemTimer::GetMs();

            task = downloader.StartTask(url, p, DLCDownloader::TaskType::FULL);

            downloader.WaitTask(task);
        }

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("new downloader 1.5 Gb download from in house server for: %f", seconds);

        TEST_VERIFY(task->status.state == DLCDownloader::TaskState::Finished);
        TEST_VERIFY(task->status.sizeTotal >= 0);
    }
};