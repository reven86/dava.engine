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

#include "DAVAEngine.h"

#include "Infrastructure/GameCore.h"
#include "Infrastructure/NewTestFramework.h"

using namespace DAVA;

static const String files[] = {
    "weird_characters",
    "de",
    "en",
    "es",
    "it",
    "ru"
};

DAVA_TESTCLASS(LocalizationTest)
{
    FilePath srcDir;
    FilePath cpyDir;

    LocalizationTest()
    {
        srcDir = "~res:/TestData/LocalizationTest/";
        cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "LocalizationTest/";

        FileSystem::Instance()->DeleteDirectory(cpyDir);
        FileSystem::Instance()->CreateDirectory(cpyDir);
    }

    ~LocalizationTest()
    {
        FileSystem::Instance()->DeleteDirectory(cpyDir);
    }

    DAVA_TEST(TestFunction)
    {
        for (size_t i = 0;i < COUNT_OF(files);++i)
        {
            FilePath srcFile = srcDir + (files[i] + ".yaml");
            FilePath cpyFile = cpyDir + (files[i] + ".yaml");

            FileSystem::Instance()->CopyFile(srcFile, cpyFile);

            LocalizationSystem* localizationSystem = LocalizationSystem::Instance();

            localizationSystem->SetCurrentLocale(files[i]);
            localizationSystem->InitWithDirectory(cpyDir);

            localizationSystem->SaveLocalizedStrings();

            localizationSystem->Cleanup();
            TEST_VERIFY_WITH_MESSAGE(FileSystem::Instance()->CompareTextFiles(srcFile, cpyFile), Format("Localization test: %s", files[i].c_str()));
        }
    }
};
