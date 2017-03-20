#include <DLCManager/DLCManager.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <DLC/Downloader/DownloadManager.h>
#include <Concurrency/Thread.h>
#include <Logger/Logger.h>
#include <Engine/Engine.h>
#include <EmbeddedWebServer.h>

#include "UnitTests/UnitTests.h"

#ifndef __DAVAENGINE_WIN_UAP__

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

        FilePath destPath = downloadedPacksDir + "superpack_for_unittests.dvpk";
        FilePath srcPath = "~res:/superpack_for_unittests.dvpk";
        if (!fs->IsFile(srcPath))
        {
            Logger::Error("no super pack file!");
            TEST_VERIFY(false);
        }

        if (!fs->CopyFile(srcPath, destPath, true))
        {
            Logger::Error("can't copy super pack for unittest from res:/");
            TEST_VERIFY(false);
            return;
        }

        try
        {
            StartEmbeddedWebServer(downloadedPacksDir.GetAbsolutePathname().c_str(), "8080");
        }
        catch (std::exception& ex)
        {
            Logger::Error("%s", ex.what());
            TEST_VERIFY(false);
            return;
        }

        String superPackUrl("http://127.0.0.1:8080/superpack_for_unittests.dvpk");

        DLCManager& dlcManager = *GetEngineContext()->dlcManager;

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

DAVA_TESTCLASS (DLCManagerFullTest)
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
                    DAVA::StopEmbeddedWebServer();
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
                    TEST_VERIFY(false);
                    downloadOfVirtualPack = true; // just go to next test
                    DAVA::StopEmbeddedWebServer();
                }
            }
        }
    }

    DAVA_TEST (TestInitializeDLCPack)
    {
        using namespace DAVA;

        DLCManager& dlcManager = *GetEngineContext()->dlcManager;

        bool getException = false;

        try
        {
            dlcManager.Initialize("C:/Windows", "http://127.0.0.1:8080/superpack_for_unittests.dvpk", DLCManager::Hints());
        }
        catch (Exception& ex)
        {
            Logger::Info("get known exception: %s", ex.what());
            getException = true;
        }

        TEST_VERIFY(getException && "can't write or no such folder exception missing");
    }

    DAVA_TEST (TestDownloadOfVirtualPack)
    {
    }
};

#endif // __DAVAENGINE_WIN_UAP__
