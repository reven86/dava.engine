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

struct FSMTest02
{
    enum State
    {
        WaitInitializationFinished,
        WaitSecondConnectAttempt
    };
    State state = WaitInitializationFinished;
    DAVA::float32 time = 0.0f;

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
                return false;
            }
        }
        break;
        case WaitSecondConnectAttempt:
        {
            // TODO how to check second connect Attemp?
        }
        break;
        }

        return time > 20.0f; // timeout
    }
};

DAVA_TESTCLASS (DLCManagerFullTest)
{
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
            dlcManager.Initialize("C:/Windows/", "http://127.0.0.1:8080/superpack_for_unittests.dvpk", DLCManager::Hints());
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

        DLCManager& dlcManager = *GetEngineContext()->dlcManager;

        FilePath res("~res:/");

        if (!StartEmbeddedWebServer(res.GetAbsolutePathname().c_str(), "8080"))
        {
            TEST_VERIFY(false && "can't start embedded web server");
        }

        dlcManager.Initialize("~doc:/UnitTests/DLCManagerTest/packs/",
                              "http://127.0.0.1:8080/superpack_for_unittests.dvpk",
                              DLCManager::Hints());
    }

    DAVA_TEST (TestServerDownDuringDownload03)
    {
    }

    DAVA_TEST (TestAddRequestAfterDisableRequesting04)
    {
    }

    DAVA_TEST (TestContinueDownloadingAfterEnableRequesting05)
    {
    }
};

#endif // __DAVAENGINE_WIN_UAP__
