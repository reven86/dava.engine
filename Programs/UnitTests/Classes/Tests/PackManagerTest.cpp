#include <PackManager/PackManager.h>
// we need include private file only to call private api in test case
#include <PackManager/Private/DLCManagerImpl.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Utils/CRC32.h>
#include <DLC/Downloader/DownloadManager.h>
#include <Core/Core.h>
#include <Platform/DeviceInfo.h>
#include <Concurrency/Thread.h>
#include <Logger/Logger.h>

#include "UnitTests/UnitTests.h"
#include "Engine/Engine.h"

class GameClient
{
public:
    GameClient(DAVA::IDLCManager& packManager_)
        : packManager(packManager_)
    {
        sigConnection = packManager.requestUpdated.Connect(this, &GameClient::OnPackStateChange);
    }
    void OnPackStateChange(const DAVA::IDLCManager::IRequest& pack)
    {
        DAVA::StringStream ss;

        ss << "pack: " << pack.GetRequestedPackName() << " change: ";
        ss << "is downloaded - " << pack.IsDownloaded();

        DAVA::Logger::Info("%s", ss.str().c_str());
    }
    DAVA::SigConnectionID sigConnection;
    DAVA::IDLCManager& packManager;
};

DAVA_TESTCLASS (PackManagerTest)
{
    DAVA_TEST (TestDownloadOfVirtualPack)
    {
        using namespace DAVA;

        Logger::Info("before init");

        FilePath downloadedPacksDir("~doc:/UnitTests/PackManagerTest/packs/");

        Logger::Info("clear dirs");

        // every time clear directory to download once again
        FileSystem::Instance()->DeleteDirectory(downloadedPacksDir);
        FileSystem::Instance()->CreateDirectory(downloadedPacksDir, true);

        String superPackUrl("http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/superpack.dvpk");

#if defined(__DAVAENGINE_COREV2__)
        IDLCManager& packManager = *Engine::Instance()->GetContext()->packManager;
#else
        IDLCManager& packManager = Core::Instance()->GetPackManager();
#endif

        FilePath fileInPack("~res:/3d/Fx/Tut_eye.sc2");

        Logger::Info("init packManager");

        try
        {
            Logger::Info("init pack manager");

            packManager.Initialize(downloadedPacksDir, superPackUrl, IDLCManager::Hints());

            Logger::Info("create game client");

            GameClient client(packManager);

            Logger::Info("wait till packManagerInitialization done");

            size_t oneSecond = 10;
            // wait till initialization done
            while (!packManager.IsInitialized() && oneSecond-- > 0)
            {
                Thread::Sleep(100);

                Logger::Info("update download manager");

                DownloadManager::Instance()->Update();

                Logger::Info("updata pack manager");

                static_cast<DLCManagerImpl*>(&packManager)->Update(0.1f);
            }

            if (!packManager.IsInitialized())
            {
                Logger::Info("can't initialize packManager(remember on build agents network disabled)");
                return;
            }

            Logger::Info("before enable requesting");

            packManager.SetRequestingEnabled(true);

            String packName = "vpack";

            Logger::Info("before request pack");

            const IDLCManager::IRequest* pack = packManager.RequestPack(packName);
            TEST_VERIFY(pack != nullptr);

            int32 maxIter = 360;

            Logger::Info("wait till pack loading");

            while ((pack != nullptr && !pack->IsDownloaded()) && maxIter-- > 0)
            {
                // wait
                Thread::Sleep(100);
                // we have to call Update() for downloadManager and packManager cause we in main thread
                DownloadManager::Instance()->Update();
                static_cast<DLCManagerImpl*>(&packManager)->Update(0.1f);
            }

            Logger::Info("finish loading pack");

            // disable test for now - on local server newer packs
            if (pack == nullptr || !pack->IsDownloaded())
            {
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
            Logger::Error("PackManagerTest failed: %s", ex.what());
            TEST_VERIFY(false);
        }
    }
};
