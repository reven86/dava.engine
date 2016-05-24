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

DAVA_TESTCLASS (LocalizationTest)
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

    DAVA_TEST (LocaleTest)
    {
        String locale = LocalizationSystem::Instance()->GetDeviceLocale();

        Logger::FrameworkDebug("Current locale is %s", locale.c_str());

        for (size_t i = 0; i < files.size(); ++i)
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

    DAVA_TEST (BiDiTest)
    {
        BiDiHelper helper;
        TextLayout layout(true);

        Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");

        FilePath filePath("~res:/TestData/LocalizationTest/bidi_test.yaml");
        YamlParser* parser = YamlParser::Create(filePath);
        SCOPE_EXIT
        {
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
