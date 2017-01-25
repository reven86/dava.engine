#include <PackManager/PackManager.h>
// we need include private file only to call private api in test case
#include <PackManager/Private/PackManagerImpl.h>
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
    GameClient(DAVA::IPackManager& packManager_)
        : packManager(packManager_)
    {
        sigConnection = packManager.packStateChanged.Connect(this, &GameClient::OnPackStateChange);
    }
    void OnPackStateChange(const DAVA::IPackManager::Pack& pack)
    {
        DAVA::StringStream ss;

        ss << "pack: " << pack.name << " change: ";
        ss << "new state - " << static_cast<unsigned>(pack.state);
        if (pack.state == DAVA::IPackManager::Pack::Status::ErrorLoading)
        {
            ss << '\n' << pack.otherErrorMsg;
        }

        DAVA::Logger::Info("%s", ss.str().c_str());
    }
    DAVA::SigConnectionID sigConnection;
    DAVA::IPackManager& packManager;
};

DAVA_TESTCLASS (PackManagerTest)
{
    DAVA_TEST (TestDownloadOfVirtualPack)
    {
        using namespace DAVA;

        Logger::Info("before init");

        String dbFileName("db_{gpu}.db.zip");
        FilePath downloadedPacksDir("~doc:/UnitTests/PackManagerTest/packs/");
        FilePath readOnlyPacksDir("~res:/TestData/PackManagerTest/packs/");

        Logger::Info("clear dirs");

        // every time clear directory to download once again
        FileSystem::Instance()->DeleteDirectory(downloadedPacksDir);
        FileSystem::Instance()->CreateDirectory(downloadedPacksDir, true);

        String superPackUrl("http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/superpack.dvpk");

        String architecture = "noname";

        Logger::Info("get gpu family");

        eGPUFamily gpu = DeviceInfo::GetGPUFamily();
        switch (gpu)
        {
        case GPU_ADRENO:
            architecture = "adreno";
            break;
        case GPU_DX11:
            architecture = "dx11";
            break;
        case GPU_MALI:
            architecture = "mali";
            break;
        case GPU_POWERVR_IOS:
            architecture = "pvr_ios";
            break;
        case GPU_POWERVR_ANDROID:
            architecture = "pvr_android";
            break;
        case GPU_TEGRA:
            architecture = "tegra";
            break;
        default:
            throw std::runtime_error("unknown gpu family");
        }

#if defined(__DAVAENGINE_COREV2__)
        IPackManager& packManager = *Engine::Instance()->GetContext()->packManager;
#else
        IPackManager& packManager = Core::Instance()->GetPackManager();
#endif

        FilePath fileInPack("~res:/3d/Fx/Tut_eye.sc2");

        dbFileName.replace(dbFileName.find("{gpu}"), 5, architecture);

        Logger::Info("init packManager");

        try
        {
            Logger::Info("init pack manager");
            FileSystem::Instance()->DeleteFile("~doc:/" + dbFileName);

            packManager.Initialize(architecture, downloadedPacksDir, "~res:/TestData/PackManagerTest/packs/" + dbFileName, superPackUrl, IPackManager::Hints());

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

                static_cast<PackManagerImpl*>(&packManager)->Update(0.1f);
            }

            if (!packManager.IsInitialized())
            {
                Logger::Info("can't initialize packManager(remember on build agents network disabled)");
                return;
            }

            Logger::Info("before enable requesting");

            packManager.EnableRequesting();

            String packName = "vpack";

            Logger::Info("before request pack");

            const IPackManager::Pack& pack = packManager.RequestPack(packName);
            if (pack.state != IPackManager::Pack::Status::Mounted)
            {
                TEST_VERIFY(pack.state == IPackManager::Pack::Status::Downloading || pack.state == IPackManager::Pack::Status::Requested);
            }

            uint32 maxIter = 360;

            Logger::Info("wait till pack loading");

            while ((pack.state == IPackManager::Pack::Status::Requested || pack.state == IPackManager::Pack::Status::Downloading) && maxIter-- > 0)
            {
                // wait
                Thread::Sleep(100);
                // we have to call Update() for downloadManager and packManager cause we in main thread
                DownloadManager::Instance()->Update();
                static_cast<PackManagerImpl*>(&packManager)->Update(0.1f);
            }

            Logger::Info("finish loading pack");

            // disable test for now - on local server newer packs
            if (pack.state != IPackManager::Pack::Status::Mounted)
            {
                return;
            }

            if (pack.state != IPackManager::Pack::Status::OtherError)
            {
                Logger::Info("check pack");

                TEST_VERIFY(pack.state == IPackManager::Pack::Status::Mounted);

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
            else
            {
                Logger::Info("check if no wifi on device");

                // if device without wifi
                const Vector<IPackManager::Pack>& allPacks = packManager.GetPacks();
                TEST_VERIFY(allPacks.at(0).name == "pack1");
                TEST_VERIFY(allPacks.at(0).downloadError == DLE_COULDNT_RESOLVE_HOST);
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
