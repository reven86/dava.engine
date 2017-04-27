#include "UnitTests/UnitTests.h"

#include <DLCManager/DLCDownloader.h>
#include <Logger/Logger.h>
#include <DLC/Downloader/DownloadManager.h>
#include <FileSystem/FileSystem.h>
#include <Time/SystemTimer.h>
#include <Utils/CRC32.h>

#include <iomanip>

class MemBufWriter final : public DAVA::DLCDownloader::IWriter
{
public:
    MemBufWriter(void* buff, size_t size)
    {
        DVASSERT(buff != nullptr);
        DVASSERT(size > 0);

        start = static_cast<char*>(buff);
        current = start;
        end = start + size;
    }

    DAVA::uint64 Save(const void* ptr, DAVA::uint64 size) override
    {
        using namespace DAVA;
        uint64 space = SpaceLeft();
        if (size > space)
        {
            DAVA_THROW(Exception, "memory corruption");
        }
        memcpy(current, ptr, static_cast<size_t>(size));
        current += size;
        return size;
    }

    DAVA::uint64 GetSeekPos() override
    {
        return current - start;
    }

    void Truncate() override
    {
        current = start;
    }

    DAVA::uint64 SpaceLeft() override
    {
        return end - current;
    }

private:
    char* start = nullptr;
    char* current = nullptr;
    char* end = nullptr;
};

static const DAVA::String URL = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk";
// "http://dl-wotblitz.wargaming.net/dlc/r11608713/3.7.0.236.dvpk"; // CDN
// "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/smart_dlc/3.7.0.236.dvpk" // local net server

DAVA_TESTCLASS (DLCDownloaderTest)
{
    const DAVA::int64 FULL_SIZE_ON_SERVER = 1618083461;

    DAVA_TEST (GetFileSizeTest)
    {
        using namespace DAVA;

        DLCDownloader* downloader = DLCDownloader::Create();
        String url = URL;
        DLCDownloader::Task* task = downloader->StartTask(url, "", DLCDownloader::TaskType::SIZE);

        downloader->WaitTask(task);

        auto& info = downloader->GetTaskInfo(task);
        auto& status = downloader->GetTaskStatus(task);

        TEST_VERIFY(info.rangeOffset == -1);
        TEST_VERIFY(info.rangeSize == -1);
        TEST_VERIFY(info.dstPath == "");
        TEST_VERIFY(info.srcUrl == url);
        TEST_VERIFY(info.timeoutSec >= 0);
        TEST_VERIFY(info.type == DLCDownloader::TaskType::SIZE);

        TEST_VERIFY(status.error.curlErr == 0);
        TEST_VERIFY(status.error.curlEasyStrErr == nullptr);
        TEST_VERIFY(status.error.curlMErr == 0);
        TEST_VERIFY(status.error.fileErrno == 0);
        TEST_VERIFY(status.sizeDownloaded == 0);
        TEST_VERIFY(status.state.Get() == DLCDownloader::TaskState::Finished);
        TEST_VERIFY(status.sizeTotal == FULL_SIZE_ON_SERVER);

        downloader->RemoveTask(task);

        DLCDownloader::Destroy(downloader);
    }

    DAVA_TEST (RangeRequestTest)
    {
        using namespace DAVA;

        std::array<char, 4> buf;
        MemBufWriter writer(buf.data(), buf.size());

        DLCDownloader* downloader = DLCDownloader::Create();
        String url = URL;
        int64 startRangeIndex = FULL_SIZE_ON_SERVER - 4;
        int64 rangeSize = 4;
        DLCDownloader::Task* downloadLast4Bytes = downloader->StartTask(url, "", DLCDownloader::TaskType::FULL, &writer, startRangeIndex, rangeSize);

        downloader->WaitTask(downloadLast4Bytes);

        auto& info = downloader->GetTaskInfo(downloadLast4Bytes);
        auto& status = downloader->GetTaskStatus(downloadLast4Bytes);

        TEST_VERIFY(info.rangeOffset == startRangeIndex);
        TEST_VERIFY(info.rangeSize == rangeSize);
        TEST_VERIFY(info.dstPath == "");
        TEST_VERIFY(info.srcUrl == url);
        TEST_VERIFY(info.timeoutSec >= 0);
        TEST_VERIFY(info.type == DLCDownloader::TaskType::FULL);

        TEST_VERIFY(status.error.curlErr == 0);
        TEST_VERIFY(status.error.curlEasyStrErr == nullptr);
        TEST_VERIFY(status.error.curlMErr == 0);
        TEST_VERIFY(status.error.fileErrno == 0);
        TEST_VERIFY(status.sizeDownloaded == 4);
        TEST_VERIFY(status.state.Get() == DLCDownloader::TaskState::Finished);
        TEST_VERIFY(status.sizeTotal == 4);

        std::array<char, 4> shouldBe{ 'D', 'V', 'P', 'K' };
        TEST_VERIFY(shouldBe == buf);

        downloader->RemoveTask(downloadLast4Bytes);

        DLCDownloader::Destroy(downloader);
    }

    DAVA_TEST (DowloadLargeFileTest)
    {
        using namespace DAVA;

        FileSystem* fs = FileSystem::Instance();
        std::unique_ptr<DLCDownloader> downloader(DLCDownloader::Create());
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

        //////----first--------------------------------------------------------
        start = SystemTimer::GetMs();

        int numOfParts = 4;

        uint32 id = dm->Download(url, pathOld, FULL);

        dm->Wait(id);

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("old downloader 1.5 Gb parts(%d) download from in house server for: %f", numOfParts, seconds);
        //// ----next-------------------------------------------------------
        {
            start = SystemTimer::GetMs();

            task = downloader->StartTask(url, p, DLCDownloader::TaskType::FULL);

            downloader->WaitTask(task);
        }

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("new downloader 1.5 Gb download from in house server for: %f", seconds);

        downloader->RemoveTask(task);

        const uint32 crc32 = 0xDE5C2B62;

        uint32 crcFromFile = CRC32::ForFile(p);

        TEST_VERIFY(crcFromFile == crc32);

        ////-----multi-files------------------------------------------------
        DLCDownloader::Task* taskSize = downloader->StartTask(url, "", DLCDownloader::TaskType::SIZE);
        downloader->WaitTask(taskSize);
        uint64 sizeTotal = downloader->GetTaskStatus(taskSize).sizeTotal;

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
            task = downloader->StartTask(url, full, DLCDownloader::TaskType::FULL, nullptr, firstIndex, nextIndex - firstIndex);
            allTasks.push_back(task);
        }

        for (auto t : allTasks)
        {
            downloader->WaitTask(t);
        }

        finish = SystemTimer::GetMs();

        seconds = (finish - start) / 1000.f;

        Logger::Info("1024 part of 1.5 Gb download from in house server for: %f", seconds);

        // free memory
        for (auto t : allTasks)
        {
            downloader->RemoveTask(t);
        }
        allTasks.clear();
    }
};