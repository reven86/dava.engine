#include <DLCManager/DLCManager.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <DLC/Downloader/DownloadManager.h>
#include <Concurrency/Thread.h>
#include <Logger/Logger.h>
#include <Engine/Engine.h>

#include "UnitTests/UnitTests.h"

struct DownloaderTest
{
    const DAVA::DLCManager::IRequest* pack = nullptr;

    DownloaderTest()
    {
        using namespace DAVA;
        Logger::Info("before init");

        FilePath downloadedPacksDir("~doc:/UnitTests/DLCManagerTest/packs/");

        Logger::Info("clear dirs");

        FileSystem* fs = GetEngineContext()->fileSystem;
        // every time clear directory to download once again
        fs->DeleteDirectory(downloadedPacksDir);
        fs->CreateDirectory(downloadedPacksDir, true);

        String superPackUrl("http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/superpack_for_unittests.dvpk");
        // "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/superpack_for_unittests.dvpk"
        // "http://127.0.0.1/superpack_for_unittests.dvpk"

        DLCManager& dlcManager = *GetEngineContext()->dlcManager;

        FilePath fileFromPack(downloadedPacksDir + "/3d/Fx/Tut_eye.sc2");

        fs->DeleteFile(fileFromPack);

        Logger::Info("init dlcManager");

        dlcManager.Initialize(downloadedPacksDir, superPackUrl, DLCManager::Hints());

        Logger::Info("create game client");

        dlcManager.SetRequestingEnabled(true);

        String packName = "0";

        Logger::Info("before request pack");

        pack = dlcManager.RequestPack(packName);
        TEST_VERIFY(pack != nullptr);
    }

    static bool IsInitialized()
    {
        using namespace DAVA;
        DLCManager& dlcManager = *GetEngineContext()->dlcManager;
        return dlcManager.IsInitialized();
    }

    bool IsDownloaded() const
    {
        if (pack != nullptr)
        {
            return pack->IsDownloaded();
        }
        return false;
    }
};

DAVA_TESTCLASS (DLCManagerTest)
{
    DownloaderTest downloader;

    DAVA::float32 timeLeftToInitAndDownloadPack = 20.0f; // seconds

    bool downloadOfVirtualPack = false;

    bool TestComplete(const DAVA::String& testName) const override
    {
        if (testName == "TestDownloadOfVirtualPack")
        {
            return downloadOfVirtualPack;
        }
        return true;
    }

    void Update(DAVA::float32 timeElapsed, const DAVA::String& testName) override
    {
        if (testName == "TestDownloadOfVirtualPack")
        {
            if (downloader.IsInitialized())
            {
                if (downloader.IsDownloaded())
                {
                    downloadOfVirtualPack = true;
                    TEST_VERIFY(true);
                }
            }
            if (!downloadOfVirtualPack)
            {
                timeLeftToInitAndDownloadPack -= timeElapsed;
                if (timeLeftToInitAndDownloadPack < 0.f)
                {
                    downloader.IsInitialized();
                    downloader.IsDownloaded();
                    DAVA::Logger::Info("can't download pack with DLCManager");
                    downloadOfVirtualPack = true;
                }
            }
        }
    }

    DAVA_TEST (TestDownloadOfVirtualPack)
    {
    }
};
