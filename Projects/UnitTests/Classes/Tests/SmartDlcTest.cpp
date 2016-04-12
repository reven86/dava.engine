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

#include <SDLC/SmartDLC.h>
#include "UnitTests/UnitTests.h"
#include "Utils/CRC32.h"

DAVA_TESTCLASS (SmartDlcTest)
{
    DAVA_TEST (TestSmartDlc)
    {
        using namespace DAVA;

        FilePath sqliteDbFile("~res:/TestData/SmartDlcTest/test.db");
        FilePath folderWithDownloadedPacks("~res:/TestData/SmartDlcTest/packs/");
        String urlToServerWithPacks("http://wargaminguberserver.net/packs");

        PackManager sdlc(sqliteDbFile, folderWithDownloadedPacks, urlToServerWithPacks);

        TEST_VERIFY(sdlc.IsProcessingEnabled() == false);

        sdlc.EnableProcessing();

        TEST_VERIFY(sdlc.IsProcessingEnabled() == true);

        sdlc.DisableProcessing();

        TEST_VERIFY(sdlc.IsProcessingEnabled() == false);

        sdlc.EnableProcessing();

        FilePath fileNotInPack("~res:/Data/no_such_file_in_any_pack.txt");

        String packID = sdlc.FindPack(fileNotInPack);
        TEST_VERIFY(packID.empty() && "no such file in any archive");

        FilePath fileInPack("~res:/TestData/Utf8Test/utf16le.txt");

        String packName = sdlc.FindPack(fileInPack);

        TEST_VERIFY(!packName.empty() && "should find file in pack");

        TEST_VERIFY(packName == String("unit_test.pak"));

        // calculate crc32 for pak
        uint64 start = SystemTimer::Instance()->AbsoluteMS();

        uint32 crc32 = CRC32::ForFile("C:\\Users\\l_chayka\\fun\\Data.zip");

        uint64 finish = SystemTimer::Instance()->AbsoluteMS();

        float timeToDoCrc32 = (finish - start) / 1000.0f;
        Logger::Info("time to calculate CRC32 for %s : %f, crc32: 0x%X", packName.c_str(), timeToDoCrc32, crc32);

        try
        {
            const PackManager::PackState& packState = sdlc.GetPackState(packName);
            TEST_VERIFY(packState.downloadProgress == 0.f);
            TEST_VERIFY(packState.name == packName);
            TEST_VERIFY(packState.priority == 0.5f);
            TEST_VERIFY(packState.state == PackManager::PackState::NotRequested);

            sdlc.DisableProcessing();

            const PackManager::PackState& requestedState = sdlc.RequestPack(packName);

            TEST_VERIFY(requestedState.state == PackManager::PackState::Requested);

            sdlc.EnableProcessing();

            auto& nextState = sdlc.RequestPack(packName, 0.1f);
            TEST_VERIFY(nextState.state == PackManager::PackState::Downloading);

            while (nextState.state == PackManager::PackState::Downloading)
            {
                // wait
                Thread::Sleep(500);
                sdlc.Update();
                Logger::Info("download progress: %d", static_cast<int32>(nextState.downloadProgress * 100));
            }

            TEST_VERIFY(nextState.state == PackManager::PackState::Mounted);

            ScopedPtr<File> file(File::Create(fileInPack, File::OPEN | File::READ));
            TEST_VERIFY(file);
            if (file)
            {
                TEST_VERIFY(file->GetSize() == 100500);
                String fileContent(file->GetSize(), '\0');
                file->Read(&fileContent[0], static_cast<uint32>(fileContent.size()));

                TEST_VERIFY(fileContent == "content of the file");
            }
        }
        catch (std::exception&)
        {
            TEST_VERIFY(false);
        }

    }
};
