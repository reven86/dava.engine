#include <fstream>

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

DAVA::FilePath documentRootDir;
const char* const localPort = "8080";

struct FSMTest02
{
    enum State
    {
        WaitInitializationFinished,
        WaitSecondConnectAttempt,
        WaitDownloadAllFourPacks,
    };
    State state = WaitInitializationFinished;
    DAVA::float32 time = 0.0f;
    DAVA::float32 waitSecondConnect = 3.0f;
    const DAVA::float32 timeout = 40.f;
    DAVA::DLCManager::Progress progressAfterInit;

    void Cleanup(DAVA::DLCManager& dlcManager)
    {
        using namespace DAVA;
        Logger::Info("%s Deinitialize()", __FUNCTION__);
        dlcManager.Deinitialize();
        Logger::Info("%s StopEmbeddedWebServer()", __FUNCTION__);
        StopEmbeddedWebServer();
        Logger::Info("%s done", __FUNCTION__);
    }

    bool Update(DAVA::float32 dt)
    {
        DAVA::DLCManager& dlcManager = *DAVA::GetEngineContext()->dlcManager;

        time += dt;

        switch (state)
        {
        case WaitInitializationFinished:
        {
            if (dlcManager.IsInitialized())
            {
                state = WaitSecondConnectAttempt;
                DAVA::StopEmbeddedWebServer();
                progressAfterInit = dlcManager.GetProgress();
                return false;
            }
        }
        break;
        case WaitSecondConnectAttempt:
        {
            // TODO how to check second connect attempt?
            TEST_VERIFY(dlcManager.IsInitialized());

            TEST_VERIFY(dlcManager.IsRequestingEnabled());

            auto currentProgress = dlcManager.GetProgress();
            TEST_VERIFY(currentProgress.alreadyDownloaded <= currentProgress.total);
            TEST_VERIFY(currentProgress.inQueue == progressAfterInit.inQueue);

            waitSecondConnect -= dt;
            if (waitSecondConnect <= 0.f)
            {
                if (!DAVA::StartEmbeddedWebServer(documentRootDir.GetAbsolutePathname().c_str(), localPort))
                {
                    TEST_VERIFY(false && "can't start server");
                }
                state = WaitDownloadAllFourPacks;
                return false;
            }
        }
        break;
        case WaitDownloadAllFourPacks:
        {
            auto currentProgress = dlcManager.GetProgress();
            TEST_VERIFY(currentProgress.alreadyDownloaded <= currentProgress.total);
            TEST_VERIFY(currentProgress.inQueue <= progressAfterInit.inQueue);
            progressAfterInit = currentProgress;

            if (currentProgress.inQueue == 0)
            {
                auto r0 = dlcManager.RequestPack("0");
                TEST_VERIFY(r0->IsDownloaded());
                auto r1 = dlcManager.RequestPack("1");
                TEST_VERIFY(r1->IsDownloaded());
                auto r2 = dlcManager.RequestPack("2");
                TEST_VERIFY(r2->IsDownloaded());
                auto r3 = dlcManager.RequestPack("3");
                TEST_VERIFY(r3->IsDownloaded());

                // now stop server for next tests
                Cleanup(dlcManager);
                return true;
            }
        }
        break;
        }

        if (time > timeout)
        {
            auto prog = dlcManager.GetProgress();

            DAVA::Logger::Error("timeout: total: %llu in_queue: %llu downloaded: %lld", prog.total, prog.inQueue, prog.alreadyDownloaded);

            DAVA::FilePath logPath(DAVA::DLCManager::Hints().logFilePath);
            DAVA::String path = logPath.GetAbsolutePathname();
            std::ifstream dlcLogFile(path.c_str());

            if (!dlcLogFile)
            {
                DAVA::Logger::Error("can't open dlcManager.log file at: %s", path.c_str());
            }
            else
            {
                DAVA::Logger::Error("begin-------dlcManager.log---------content");
                for (DAVA::String str; getline(dlcLogFile, str);)
                {
                    DAVA::Logger::Error("%s", str.c_str());
                }
                DAVA::Logger::Error("end-------dlcManager.log---------content");
            }

            Cleanup(dlcManager);

            TEST_VERIFY(false && "time out wait second connection");
            return true;
        }

        return false;
    }
};

DAVA_TESTCLASS (DLCManagerFullTest)
{
    //BEGIN_FILES_COVERED_BY_TESTS()
    //DECLARE_COVERED_FILES("DLCManagerImpl.cpp")
    //DECLARE_COVERED_FILES("PackRequest.cpp")
    //DECLARE_COVERED_FILES("RequestManager.cpp")
    //END_FILES_COVERED_BY_TESTS()

    FSMTest02 fsm02;
    bool TestAfterInitStopServer02_done = false;

    bool TestComplete(const DAVA::String& testName) const override
    {
        if (testName == "TestAfterInitStopServer02")
        {
            return TestAfterInitStopServer02_done;
        }
        return true;
    }

    void Update(DAVA::float32 timeElapsed, const DAVA::String& testName) override
    {
        if (testName == "TestAfterInitStopServer02")
        {
            TestAfterInitStopServer02_done = fsm02.Update(timeElapsed);
        }
    }

    DAVA_TEST (TestInitializeBadFolder01)
    {
        using namespace DAVA;

        DLCManager& dlcManager = *GetEngineContext()->dlcManager;

        bool getException = false;

        try
        {
#ifdef __DAVAENGINE_WINDOWS__
            const char* cant_write_dir = "C:/Windows/"; // system dir
#else
            const char* cant_write_dir = "/"; // root dir
#endif
            dlcManager.Initialize(cant_write_dir, "http://127.0.0.1:8080/superpack_for_unittests.dvpk", DLCManager::Hints());
        }
        catch (Exception& ex)
        {
            Logger::Info("get known exception: %s", ex.what());
            getException = true;
        }

        TEST_VERIFY(getException && "can't write or no such folder exception missing");

        dlcManager.Deinitialize();
    }

    DAVA_TEST (TestAfterInitStopServer02)
    {
        using namespace DAVA;

        documentRootDir = "~doc:/";

        Logger::Info("First part of TestAfterInitStopServer02 started");

        DLCManager& dlcManager = *GetEngineContext()->dlcManager;

        const DLCManager::IRequest* r = dlcManager.RequestPack("1"); // pack "1" have one dependent pack "0"
        TEST_VERIFY(r != nullptr);

        FileSystem* fs = FileSystem::Instance();

        FilePath destPath = documentRootDir + "superpack_for_unittests.dvpk";
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

        if (!StartEmbeddedWebServer(documentRootDir.GetAbsolutePathname().c_str(), localPort))
        {
            TEST_VERIFY(false && "can't start embedded web server");
            return;
        }

        auto hints = DLCManager::Hints();
        hints.retryConnectMilliseconds = 3000;

        FilePath packDir("~doc:/UnitTests/DLCManagerTest/packs/");
        FileSystem::Instance()->DeleteDirectory(packDir, true);

        try
        {
            dlcManager.Initialize(packDir,
                                  "http://127.0.0.1:8080/superpack_for_unittests.dvpk",
                                  hints);
            Logger::Info("Initialize called no exception");
        }
        catch (std::exception& ex)
        {
            Logger::Error("error: %s", ex.what());
            TEST_VERIFY(false && "can't initialize dlcManager");
        }

        auto request = dlcManager.RequestPack("3"); // pack "3" depends on "0, 1, 2" packs
        TEST_VERIFY(request != nullptr);
        TEST_VERIFY(dlcManager.IsRequestingEnabled());
        Logger::Info("First part of TestAfterInitStopServer02 finished");
    }
};

#endif // __DAVAENGINE_WIN_UAP__
