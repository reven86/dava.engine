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


#include "LocalizationTest.h"
#include "Render\2D\TextLayout.h"

static const String files[] = {
	"weird_characters",
	"de",
	"en",
	"es",
	"it",
	"ru"
};

LocalizationTest::LocalizationTest()
:	TestTemplate<LocalizationTest>("LocalizationTest")
{
	currentTest = FIRST_TEST;

	for (int32 i = FIRST_TEST; i < FIRST_TEST + TEST_COUNT; ++i)
	{
		RegisterFunction(this, &LocalizationTest::TestFunction, Format("Localization test of %s", files[i].c_str()), NULL);
	}

	srcDir = "~res:/TestData/LocalizationTest/";
	cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "LocalizationTest/";

	FileSystem::Instance()->DeleteDirectory(cpyDir);
	FileSystem::Instance()->CreateDirectory(cpyDir);

    RegisterFunction(this, &LocalizationTest::BiDiTest, "BiDiTest", NULL);
}

void LocalizationTest::LoadResources()
{
}

void LocalizationTest::UnloadResources()
{
    FileSystem::Instance()->DeleteDirectory(cpyDir);
}

void LocalizationTest::Draw(const DAVA::UIGeometricData &geometricData)
{
}

void LocalizationTest::TestFunction(TestTemplate<LocalizationTest>::PerfFuncData *data)
{
	FilePath srcFile = srcDir + (files[currentTest] + ".yaml");
	FilePath cpyFile = cpyDir + (files[currentTest] + ".yaml");

	FileSystem::Instance()->CopyFile(srcFile, cpyFile);

	LocalizationSystem* localizationSystem = LocalizationSystem::Instance();

	localizationSystem->SetCurrentLocale(files[currentTest]);
	localizationSystem->InitWithDirectory(cpyDir);

	localizationSystem->SaveLocalizedStrings();

	localizationSystem->Cleanup();

    bool res = FileSystem::Instance()->CompareTextFiles(srcFile, cpyFile);

	String s = Format("Localization test %d: %s - %s", currentTest, files[currentTest].c_str(), (res ? "passed" : "fail"));
	Logger::Debug(s.c_str());

	data->testData.message = s;
	TEST_VERIFY(res);

	++currentTest;
}

void LocalizationTest::BiDiTest(TestTemplate<LocalizationTest>::PerfFuncData * data)
{
    TextLayout layout(true);

    auto font = FTFont::Create("~res:/Fonts/korinna.ttf");

    FilePath filePath("~res:/TestData/LocalizationTest/bidi_test.yaml");
    auto parser = YamlParser::Create(filePath);
    SCOPE_EXIT
    { 
        SafeRelease(parser); 
        SafeRelease(font);
    };

    if (!parser)
    {
        Logger::Error("Failed to open yaml file: %s", filePath.GetAbsolutePathname().c_str());
        TEST_VERIFY(false);
        return;
    }

    auto rootNode = parser->GetRootNode();
    if (!rootNode)
    {
        Logger::Error("yaml file: %s is empty", filePath.GetAbsolutePathname().c_str());
        TEST_VERIFY(false);
        return;
    }

    auto cnt = rootNode->GetCount();
    for (auto k = 0U; k < cnt; ++k)
    {
        auto node = rootNode->Get(k);
        auto inputNode = node->Get("input");
        auto visualNode = node->Get("visual");

        DVASSERT_MSG(inputNode, "input node is empty");
        DVASSERT_MSG(visualNode, "visual node is empty");

        auto input = inputNode->AsWString();
        auto visual = visualNode->AsWString();
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

        auto res = visual == visual_work;

        data->testData.message = Format("Localization::BiDi test %d: %s", k, (res ? "passed" : "fail"));
        Logger::Debug(data->testData.message.c_str());

        TEST_VERIFY(res);
    }
    
}
