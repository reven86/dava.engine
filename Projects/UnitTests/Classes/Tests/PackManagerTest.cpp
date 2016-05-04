/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include <PackManager/PackManager.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Utils/CRC32.h>
#include <DLC/Downloader/DownloadManager.h>
#include <Core/Core.h>

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
            break;
        case DAVA::PackManager::Pack::Change::DownloadProgress:
            ss << "download progress - " << pack.downloadProgress;
            break;
        case DAVA::PackManager::Pack::Change::Priority:
            ss << "new priority - " << pack.priority;
            break;
        }

        DAVA::Logger::Debug("%s", ss.str().c_str());
    }
    DAVA::SigConnectionID sigConnection;
    DAVA::PackManager& packManager;
};

DAVA_TESTCLASS (PackManagerTest)
{
    DAVA_TEST (TestDownloadOfVirtualPack)
    {
        using namespace DAVA;

        FilePath sqliteDbFile("~res:/TestData/SmartDlcTest/test.db");
        FilePath folderWithDownloadedPacks("~doc:/PackManagerTest/packs/");

        // every time clear directory to download once again
        FileSystem::Instance()->DeleteDirectory(folderWithDownloadedPacks);
        FileSystem::Instance()->CreateDirectory(folderWithDownloadedPacks, true);

        String urlToServerWithPacks("http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/");

        PackManager& packManager = Core::Instance()->GetPackManager();
        packManager.Initialize(sqliteDbFile, folderWithDownloadedPacks, urlToServerWithPacks);

        GameClient client(packManager);

        packManager.EnableProcessing();

        FilePath fileNotInPack("~res:/Data/no_such_file_in_any_pack.txt");

        String packID = packManager.FindPack(fileNotInPack);
        TEST_VERIFY(packID.empty() && "no such file in any archive");

        FilePath fileInPack("~res:/TestData/Utf8Test/utf16le.txt");

        String packName = packManager.FindPack(fileInPack);

        TEST_VERIFY(!packName.empty() && "should find file in pack");

        TEST_VERIFY(packName == String("unit_test.pak"));

        try
        {
            packName = "unit_test.pak";
            const PackManager::Pack& packState = packManager.GetPack(packName);
            TEST_VERIFY(packState.name == packName);

            auto& nextState = packManager.RequestPack(packName, 0.1f);
            if (nextState.state != PackManager::Pack::Mounted)
            {
                TEST_VERIFY(nextState.state == PackManager::Pack::Downloading || nextState.state == PackManager::Pack::Requested);
            }

            uint32 maxIter = 30;

            while ((nextState.state == PackManager::Pack::Requested || nextState.state == PackManager::Pack::Downloading) && maxIter-- > 0)
            {
                // wait
                Thread::Sleep(100);
                // we have to call Update() for downloadManager and packManager cause we in main thread
                DownloadManager::Instance()->Update();
                packManager.Update();
            }

            TEST_VERIFY(nextState.state == PackManager::Pack::Mounted);

            ScopedPtr<File> file(File::Create(fileInPack, File::OPEN | File::READ));
            TEST_VERIFY(file);
            if (file)
            {
                TEST_VERIFY(file->GetSize() == 138); // utf16le.txt - 138 byte

                String fileContent(file->GetSize(), '\0');
                file->Read(&fileContent[0], static_cast<uint32>(fileContent.size()));

                uint32 crc32 = CRC32::ForBuffer(fileContent.data(), static_cast<uint32>(fileContent.size()));

                TEST_VERIFY(crc32 == 0x60076e58);
            }
        }
        catch (std::exception& ex)
        {
            Logger::Error("%s", ex.what());
            TEST_VERIFY(false);
        }
    }
};
