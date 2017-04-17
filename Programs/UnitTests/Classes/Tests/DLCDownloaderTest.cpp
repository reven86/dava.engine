#include "UnitTests/UnitTests.h"

#include "DLCManager/Private/DLCDownloaderImpl.h"

#include <Logger/Logger.h>
#include <Time/SystemTimer.h>
#include <DLC/Downloader/DownloadManager.h>
#include <FileSystem/FileSystem.h>

#include <iomanip>

static const DAVA::String URL = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk";
// "http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk"; // CDN
// "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk" // local net server

DAVA_TESTCLASS (DLCDownloaderTest)
{
    DAVA_TEST (GetFileSizeTest)
    {
        using namespace DAVA;

        DLCDownloaderImpl downloader;
        String url = URL;
        DLCDownloader::Task* task = downloader.StartTask(url, "", DLCDownloader::TaskType::SIZE);

        downloader.WaitTask(task);

        TEST_VERIFY(task->status.state == DLCDownloader::TaskState::Finished);
        TEST_VERIFY(task->status.sizeTotal >= 0);
    }

    DAVA_TEST (DowloadLargeFileTest)
    {
        using namespace DAVA;

        FileSystem* fs = FileSystem::Instance();
        DLCDownloaderImpl downloader;
        String url = URL;
        FilePath path("~doc:/big_tmp_file_from_server.remove.me");
        fs->DeleteFile(path);
        String p = path.GetAbsolutePathname();
        int64 start = 0;
        DLCDownloader::Task* task = nullptr;
        int64 finish = 0;
        float seconds = 0.f;

        DownloadManager* dm = DownloadManager::Instance();

        FilePath pathOld("~doc:/big_tmp_file_from_server.old.remove.me");
        fs->DeleteFile(pathOld);

        ////----first--------------------------------------------------------
        start = SystemTimer::GetMs();

        int numOfParts = 4;

        uint32 id = dm->Download(url, pathOld, FULL);

        dm->Wait(id);

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("old downloader 1.5 Gb parts(%d) download from in house server for: %f", numOfParts, seconds);
        // ----next-------------------------------------------------------
        {
            start = SystemTimer::GetMs();

            task = downloader.StartTask(url, p, DLCDownloader::TaskType::FULL);

            downloader.WaitTask(task);
        }

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("new downloader 1.5 Gb download from in house server for: %f", seconds);

        //-----multi-files------------------------------------------------
        DLCDownloader::Task* taskSize = downloader.StartTask(url, "", DLCDownloader::TaskType::SIZE);
        downloader.WaitTask(taskSize);
        uint64 sizeTotal = taskSize->status.sizeTotal;

        uint64 firstIndex = 0;
        uint64 nextIndex = 0;
        const uint64 lastIndex = sizeTotal - 1;

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
            ss << "part_" << std::setw(5) << std::setfill('0') << i << '_' << firstIndex << '-' << nextIndex;
            String fileName = ss.str() + ".part";
            FilePath pathFull = dir + fileName;
            String full = pathFull.GetAbsolutePathname();
            task = downloader.StartTask(url, full, DLCDownloader::TaskType::FULL, nullptr, firstIndex, nextIndex - firstIndex);
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