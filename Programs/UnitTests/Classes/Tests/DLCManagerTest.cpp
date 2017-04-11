#include <DLCManager/DLCManager.h>
// we need include private file only to call private api in test case
#include <DLCManager/Private/DLCManagerImpl.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Utils/CRC32.h>
#include <DLC/Downloader/DownloadManager.h>
#include <Concurrency/Thread.h>
#include <Logger/Logger.h>

#include "UnitTests/UnitTests.h"

class GameClient
{
public:
    GameClient(DAVA::DLCManager& packManager_)
        : dlcManager(packManager_)
    {
        sigConnection = dlcManager.requestUpdated.Connect(this, &GameClient::OnPackStateChange);
    }
    void OnPackStateChange(const DAVA::DLCManager::IRequest& pack)
    {
        DAVA::StringStream ss;

        ss << "pack: " << pack.GetRequestedPackName() << " change: ";
        ss << "is downloaded - " << pack.IsDownloaded();

        DAVA::Logger::Info("%s", ss.str().c_str());
    }
    DAVA::SigConnectionID sigConnection;
    DAVA::DLCManager& dlcManager;
};

DAVA_TESTCLASS (DLCManagerTest)
{
    DAVA_TEST (TestDownloadOfVirtualPack)
    {
        using namespace DAVA;

        Logger::Info("before init");

        FilePath downloadedPacksDir("~doc:/UnitTests/DLCManagerTest/packs/");

        Logger::Info("clear dirs");

        FileSystem* fileSystem = GetEngineContext()->fileSystem;
        // every time clear directory to download once again
        fileSystem->DeleteDirectory(downloadedPacksDir);
        fileSystem->CreateDirectory(downloadedPacksDir, true);

        String superPackUrl("http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/superpack.dvpk");

        DLCManager& dlcManager = *GetEngineContext()->dlcManager;

        FilePath fileInPack("~res:/3d/Fx/Tut_eye.sc2");

        Logger::Info("init dlcManager");

        try
        {
            Logger::Info("init pack manager");

            dlcManager.Initialize(downloadedPacksDir, superPackUrl, DLCManager::Hints());

            Logger::Info("create game client");

            GameClient client(dlcManager);

            Logger::Info("wait till packManagerInitialization done");

            size_t oneSecond = 10;
            // wait till initialization done
            while (!dlcManager.IsInitialized() && oneSecond-- > 0)
            {
                Thread::Sleep(100);

                Logger::Info("update download manager");

                DownloadManager::Instance()->Update();

                Logger::Info("updata pack manager");

                static_cast<DLCManagerImpl*>(&dlcManager)->Update(0.1f);
            }

            if (!dlcManager.IsInitialized())
            {
                Logger::Info("can't initialize dlcManager(remember on build agents network disabled)");
                dlcManager.Deinitialize();
                return;
            }

            Logger::Info("before enable requesting");

            dlcManager.SetRequestingEnabled(true);

            String packName = "vpack";

            Logger::Info("before request pack");

            const DLCManager::IRequest* pack = dlcManager.RequestPack(packName);
            TEST_VERIFY(pack != nullptr);

            int32 maxIter = 36;

            Logger::Info("wait till pack loading");

            while ((pack != nullptr && !pack->IsDownloaded()) && maxIter-- > 0)
            {
                // wait
                Thread::Sleep(100);
                // we have to call Update() for downloadManager and dlcManager cause we in main thread
                DownloadManager::Instance()->Update();
                static_cast<DLCManagerImpl*>(&dlcManager)->Update(0.1f);
            }

            Logger::Info("finish loading pack");

            // disable test for now - on local server newer packs
            if (pack == nullptr || !pack->IsDownloaded())
            {
                dlcManager.Deinitialize();
                return;
            }

            if (pack->IsDownloaded())
            {
                Logger::Info("check pack TODO implement it(need regenerate new test data)");

                ScopedPtr<File> file(File::Create(fileInPack, File::OPEN | File::READ));
                TEST_VERIFY(file);
                if (file)
                {
                    String fileContent(static_cast<size_t>(file->GetSize()), '\0');
                    file->Read(&fileContent[0], static_cast<uint32>(fileContent.size()));

                    uint32 crc32 = CRC32::ForBuffer(fileContent.data(), static_cast<uint32>(fileContent.size()));

                    TEST_VERIFY(crc32 == 0x4a2039c8); // crc32 for monkey.sc2
                }
            }

            Logger::Info("done test");
        }
        catch (std::exception& ex)
        {
            Logger::Error("DLCManagerTest failed: %s", ex.what());
            TEST_VERIFY(false);
        }

        dlcManager.Deinitialize();
    }
};
