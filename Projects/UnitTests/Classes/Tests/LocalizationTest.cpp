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
#include "UnitTests/UnitTests.h"

#include "Utils/BiDiHelper.h"
#include "Render/2D/TextLayout.h"

#include <float.h>

using namespace DAVA;

static const Vector<String> files = {
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

    DAVA_TEST(LocaleTest)
    {
        for (size_t i = 0;i < files.size();++i)
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

    DAVA_TEST(BiDiTest)
    {
        BiDiHelper helper;
        TextLayout layout(true);

        Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");

        FilePath filePath("~res:/TestData/LocalizationTest/bidi_test.yaml");
        YamlParser* parser = YamlParser::Create(filePath);
        SCOPE_EXIT {
            SafeRelease(parser);
            SafeRelease(font);
        };

        TEST_VERIFY_WITH_MESSAGE(parser != nullptr, Format("Failed to open yaml file: %s", filePath.GetAbsolutePathname().c_str()));
        if (parser == nullptr)
            return;

        YamlNode* rootNode = parser->GetRootNode();
        TEST_VERIFY_WITH_MESSAGE(rootNode != nullptr, Format("Empty YAML file: %s", filePath.GetAbsolutePathname().c_str()));
        if (rootNode == nullptr)
            return;

        uint32 cnt = rootNode->GetCount();
        for (uint32 k = 0; k < cnt; ++k)
        {
            const YamlNode* node = rootNode->Get(k);
            const YamlNode* inputNode = node->Get("input");
            const YamlNode* visualNode = node->Get("visual");

            TEST_VERIFY_WITH_MESSAGE(inputNode != nullptr, Format("YamlNode %d: input node is empty", k));
            TEST_VERIFY_WITH_MESSAGE(visualNode != nullptr, Format("YamlNode %d: visual node is empty", k));
            if (inputNode == nullptr || visualNode == nullptr)
                break;

            WideString input = inputNode->AsWString();
            WideString visual = visualNode->AsWString();
            WideString visual_work;

            layout.Reset(input, *font);
            while (!layout.IsEndOfText())
            {
                layout.NextByWords(FLT_MAX);
                visual_work += layout.GetVisualLine(!layout.IsEndOfText());
                if (!layout.IsEndOfText())
                {
                    // Paste linebreak for comparing splinted strings and string from config
                    visual_work += L"\n";
                }
            }
            TEST_VERIFY_WITH_MESSAGE(visual == visual_work, Format("YamlNode index: %d", k));
        }
    }
};
