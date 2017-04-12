#include "UnitTests/UnitTests.h"

#include "DLCManager/Private/DLCDownloaderImpl.h"

#include <Logger/Logger.h>
#include <Time/SystemTimer.h>
#include <DLC/Downloader/DownloadManager.h>
#include <FileSystem/FileSystem.h>

#include <iomanip>

DAVA_TESTCLASS (DLCDownloaderTest)
{
    DAVA_TEST (GetFileSizeTest)
    {
        using namespace DAVA;

        DLCDownloaderImpl downloader;
        String url = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.245.dvpk";
        DLCDownloader::Task* task = downloader.StartTask(url, "", DLCDownloader::TaskType::SIZE);

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

        //FilePath pathOld("~doc:/big_tmp_file_from_server.old.remove.me");

        ////----first--------------------------------------------------------
        //start = SystemTimer::GetMs();

        //uint32 id = dm->Download(url, pathOld);

        //dm->Wait(id);

        //finish = SystemTimer::GetMs();

        //seconds = (finish - start) / 1000.f;

        //Logger::Info("old downloader 1.5 Gb download from in house server for: %f", seconds);
        // ----next-------------------------------------------------------
        //{
        //    start = SystemTimer::GetMs();

        //    task = downloader.StartTask(url, p, DLCDownloader::TaskType::FULL);

        //    downloader.WaitTask(task);
        //}

        //finish = SystemTimer::GetMs();

        //seconds = (finish - start) / 1000.f;

        //Logger::Info("new downloader 1.5 Gb download from in house server for: %f", seconds);

        //TEST_VERIFY(task->status.state == DLCDownloader::TaskState::Finished);
        //TEST_VERIFY(task->status.sizeTotal >= 0);
        //-----multi-files------------------------------------------------
        DLCDownloader::Task* taskSize = downloader.StartTask(url, "", DLCDownloader::TaskType::SIZE);
        downloader.WaitTask(taskSize);
        uint64 sizeTotal = taskSize->status.sizeTotal;

        uint64 firstIndex = 0;
        uint64 nextIndex = 0;
        const uint64 lastIndex = sizeTotal - 1;

        FileSystem* fs = FileSystem::Instance();
        FilePath dir("~doc:/multy_tmp/");
        fs->DeleteDirectory(dir, true);
        fs->CreateDirectory(dir);

        const size_t numAll = 1024;
        const size_t onePart = static_cast<size_t>(sizeTotal / numAll);

        nextIndex += onePart;

        Vector<DLCDownloader::Task*> allTasks;
        allTasks.reserve(numAll);

        start = SystemTimer::GetMs();

        for (size_t i = 0; i < numAll; ++i, firstIndex += onePart, nextIndex += onePart)
        {
            StringStream ss;
            ss << "part_" << std::setw(5) << std::setfill('0') << i << '_' << firstIndex << '-';
            uint64 endIndex = 0;
            if (nextIndex <= lastIndex)
            {
                endIndex = nextIndex;
            }
            else
            {
                endIndex = lastIndex;
            }
            ss << endIndex;
            String fileName = ss.str() + ".part";
            FilePath pathFull = dir + fileName;
            String full = pathFull.GetAbsolutePathname();
            task = downloader.StartTask(url, full, DLCDownloader::TaskType::FULL, nullptr, firstIndex, endIndex);
            allTasks.push_back(task);
        }

        for (auto t : allTasks)
        {
            downloader.WaitTask(t);
        }

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("1024 part of 1.5 Gb download from in house server for: %f", seconds);
    }
};