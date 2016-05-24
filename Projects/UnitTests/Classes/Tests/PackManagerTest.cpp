#include <PackManager/PackManager.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Utils/CRC32.h>
#include <DLC/Downloader/DownloadManager.h>
#include <Core/Core.h>
#include <Platform/DeviceInfo.h>

#include "UnitTests/UnitTests.h"

class GameClient
{
public:
    GameClient(DAVA::PackManager& packManager_)
        : packManager(packManager_)
    {
        sigConnection = packManager.onPackStateChanged.Connect(this, &GameClient::OnPackStateChange);
    }
    void OnPackStateChange(const DAVA::PackManager::Pack& pack, DAVA::PackManager::Pack::Change change)
    {
        DAVA::StringStream ss;

        ss << "pack: " << pack.name << " change: ";

        switch (change)
        {
        case DAVA::PackManager::Pack::Change::State:
            ss << "new state - " << static_cast<unsigned>(pack.state);
            if (pack.state == DAVA::PackManager::Pack::Status::ErrorLoading)
            {
                ss << '\n' << pack.otherErrorMsg;
            }
            break;
        case DAVA::PackManager::Pack::Change::DownloadProgress:
            ss << "download progress - " << pack.downloadProgress;
            break;
        case DAVA::PackManager::Pack::Change::Priority:
            ss << "new priority - " << pack.priority;
            break;
        }

        DAVA::Logger::FrameworkDebug("%s", ss.str().c_str());
    }
    DAVA::SigConnectionID sigConnection;
    DAVA::PackManager& packManager;
};

DAVA_TESTCLASS (PackManagerTest)
{
    DAVA_TEST (TestDownloadOfVirtualPack)
    {
        using namespace DAVA;

        String dbFile("~res:/TestData/PackManagerTest/packs/testbed_{gpu}.db");
        FilePath folderWithDownloadedPacks("~doc:/UnitTests/PackManagerTest/packs/");

        // every time clear directory to download once again
        FileSystem::Instance()->DeleteDirectory(folderWithDownloadedPacks);
        FileSystem::Instance()->CreateDirectory(folderWithDownloadedPacks, true);

        String commonPacksUrl("http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/common/");
        String gpuPacksUrl("http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/{gpu}/");

        String gpuName = "noname";

        eGPUFamily gpu = DeviceInfo::GetGPUFamily();
        switch (gpu)
        {
        case GPU_ADRENO:
            gpuName = "adreno";
            break;
        case GPU_DX11:
            gpuName = "dx11";
            break;
        case GPU_MALI:
            gpuName = "mali";
            break;
        case GPU_POWERVR_IOS:
            gpuName = "pvr_ios";
            break;
        case GPU_POWERVR_ANDROID:
            gpuName = "pvr_android";
            break;
        case GPU_TEGRA:
            gpuName = "tegra";
            break;
        default:
            throw std::runtime_error("unknown gpu famili");
        }

        dbFile.replace(dbFile.find("{gpu}"), 5, gpuName);
        gpuPacksUrl.replace(gpuPacksUrl.find("{gpu}"), 5, gpuName);

        FilePath sqliteDbFile(dbFile);

        PackManager& packManager = Core::Instance()->GetPackManager();

        FilePath fileInPack("~res:/Data/3d/Objects/monkey.sc2");

        try
        {
            packManager.Initialize(sqliteDbFile,
                                   folderWithDownloadedPacks,
                                   commonPacksUrl,
                                   gpuPacksUrl);

            GameClient client(packManager);

            packManager.EnableProcessing();

            String packName = "vpack";

            const PackManager::Pack& pack = packManager.RequestPack(packName);
            if (pack.state != PackManager::Pack::Status::Mounted)
            {
                TEST_VERIFY(pack.state == PackManager::Pack::Status::Downloading || pack.state == PackManager::Pack::Status::Requested);
            }

            uint32 maxIter = 360;

            while ((pack.state == PackManager::Pack::Status::Requested || pack.state == PackManager::Pack::Status::Downloading) && maxIter-- > 0)
            {
                // wait
                Thread::Sleep(100);
                // we have to call Update() for downloadManager and packManager cause we in main thread
                DownloadManager::Instance()->Update();
                packManager.Update();
            }

            if (pack.state != PackManager::Pack::Status::ErrorLoading)
            {
                TEST_VERIFY(pack.state == PackManager::Pack::Status::Mounted);

                ScopedPtr<File> file(File::Create(fileInPack, File::OPEN | File::READ));
                TEST_VERIFY(file);
                if (file)
                {
                    String fileContent(file->GetSize(), '\0');
                    file->Read(&fileContent[0], static_cast<uint32>(fileContent.size()));

                    uint32 crc32 = CRC32::ForBuffer(fileContent.data(), static_cast<uint32>(fileContent.size()));

                    TEST_VERIFY(crc32 == 0xc8101bca); // crc32 for monkey.sc2
                }
            }
            else
            {
                // if device without wifi
                const Vector<PackManager::Pack>& allPacks = packManager.GetPacks();
                TEST_VERIFY(allPacks.at(0).name == "pack1");
                TEST_VERIFY(allPacks.at(0).downloadError == DLE_COULDNT_RESOLVE_HOST);
            }
        }
        catch (std::exception& ex)
        {
            Logger::Error("%s", ex.what());
            TEST_VERIFY(false);
        }
    }
};
