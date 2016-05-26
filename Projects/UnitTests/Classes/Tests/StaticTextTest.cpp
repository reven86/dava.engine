#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "Utils/StringUtils.h"

using namespace DAVA;

static WideString TEST_DATA = L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES";
static struct FittingTestInfo
{
    int32 fitting;
    int32 align;
    WideString result;
} testData[] = {
    { 0, ALIGN_LEFT, L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTIN" },
    { 0, ALIGN_HCENTER, L"DAVA CONSULTING, LLC AND CONTRIBUTORS AS IS AND" },
    { 0, ALIGN_RIGHT, L"ORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES" },
    { TextBlock::FITTING_POINTS, ALIGN_LEFT, L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTI..." },
    { TextBlock::FITTING_POINTS, ALIGN_HCENTER, L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTI..." },
    { TextBlock::FITTING_POINTS, ALIGN_RIGHT, L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTI..." },
    { TextBlock::FITTING_REDUCE, ALIGN_LEFT, TEST_DATA },
    { TextBlock::FITTING_REDUCE, ALIGN_HCENTER, TEST_DATA },
    { TextBlock::FITTING_REDUCE, ALIGN_RIGHT, TEST_DATA },
};

DAVA_TESTCLASS (StaticTextTest)
{
    UIStaticText* staticText = nullptr;
    Font* font = nullptr;

    StaticTextTest()
    {
        staticText = new UIStaticText();
        staticText->SetRect(Rect(10.f, 10.f, 400.f, 200.f));
        font = FTFont::Create("~res:/Fonts/korinna.ttf");
        staticText->SetFont(font);
    }

    ~StaticTextTest()
    {
        SafeRelease(staticText);
        SafeRelease(font);
    }

    DAVA_TEST (SplitByWords)
    {
        staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
        Vector<WideString> resultStrings;
        resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY THE DAVA");
        resultStrings.push_back(L"CONSULTING, LLC AND CONTRIBUTORS AS IS AND ANY");
        resultStrings.push_back(L"EXPRESS OR IMPLIED WARRANTIES");

        staticText->SetMultiline(true);

        Vector<WideString> strings = staticText->GetMultilineStrings();
        bool testSuccess = true;

        if (strings.size() != resultStrings.size())
        {
            testSuccess = false;
        }
        else
        {
            Vector<WideString>::iterator it = strings.begin();
            Vector<WideString>::iterator itResult = resultStrings.begin();
            while (it != strings.end() && itResult != resultStrings.end())
            {
                if (*it != *itResult)
                    testSuccess = false;
                ++it;
                ++itResult;
            }
        }

        TEST_VERIFY(testSuccess);
    }

    DAVA_TEST (SplitBySymbols)
    {
        staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
        Vector<WideString> resultStrings;
        resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTIN");
        resultStrings.push_back(L"G, LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR");
        resultStrings.push_back(L" IMPLIED WARRANTIES");

        staticText->SetMultiline(true, true);

        Vector<WideString> strings = staticText->GetMultilineStrings();

        bool testSuccess = true;

        if (strings.size() != resultStrings.size())
        {
            testSuccess = false;
        }
        else
        {
            Vector<WideString>::iterator it = strings.begin();
            Vector<WideString>::iterator itResult = resultStrings.begin();
            while (it != strings.end() && itResult != resultStrings.end())
            {
                if (*it != *itResult)
                    testSuccess = false;
                ++it;
                ++itResult;
            }
        }

        TEST_VERIFY(testSuccess);
    }

    DAVA_TEST (SplitByWordsWithNewLine)
    {
        staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY\nTHE DAVA CONSULTING\nLLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
        Vector<WideString> resultStrings;
        resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY");
        resultStrings.push_back(L"THE DAVA CONSULTING");
        resultStrings.push_back(L"LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR");
        resultStrings.push_back(L"IMPLIED WARRANTIES");

        staticText->SetMultiline(true);

        Vector<WideString> strings = staticText->GetMultilineStrings();

        bool testSuccess = true;

        if (strings.size() != resultStrings.size())
        {
            testSuccess = false;
        }
        else
        {
            Vector<WideString>::iterator it = strings.begin();
            Vector<WideString>::iterator itResult = resultStrings.begin();
            while (it != strings.end() && itResult != resultStrings.end())
            {
                if (*it != *itResult)
                    testSuccess = false;
                ++it;
                ++itResult;
            }
        }

        TEST_VERIFY(testSuccess);
    }

    DAVA_TEST (SplitBySymbolsWithNewLine)
    {
        staticText->SetText(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING,\nLLC AND\nCONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
        Vector<WideString> resultStrings;
        resultStrings.push_back(L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTIN");
        resultStrings.push_back(L"G,");
        resultStrings.push_back(L"LLC AND");
        resultStrings.push_back(L"CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED W");
        resultStrings.push_back(L"ARRANTIES");

        staticText->SetMultiline(true, true);

        Vector<WideString> strings = staticText->GetMultilineStrings();

        bool testSuccess = true;

        if (strings.size() != resultStrings.size())
        {
            testSuccess = false;
        }
        else
        {
            Vector<WideString>::iterator it = strings.begin();
            Vector<WideString>::iterator itResult = resultStrings.begin();
            while (it != strings.end() && itResult != resultStrings.end())
            {
                if (*it != *itResult)
                    testSuccess = false;
                ++it;
                ++itResult;
            }
        }

        TEST_VERIFY(testSuccess);
    }

    DAVA_TEST (SplitAndTrimTest)
    {
        staticText->SetText(L"  THIS SOFTWARE IS   \u00A0   PROVIDED BY   \u00A0   \n  THE DAVA CONSULTING\n LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES");
        Vector<WideString> resultStrings;
        resultStrings.push_back(L"  THIS SOFTWARE IS       PROVIDED BY");
        resultStrings.push_back(L"  THE DAVA CONSULTING");
        resultStrings.push_back(L" LLC AND CONTRIBUTORS AS IS AND ANY EXPRESS OR");
        resultStrings.push_back(L"IMPLIED WARRANTIES");

        staticText->SetMultiline(true);

        Vector<WideString> strings = staticText->GetMultilineStrings();

        bool testSuccess = true;

        if (strings.size() != resultStrings.size())
        {
            testSuccess = false;
        }
        else
        {
            Vector<WideString>::iterator it = strings.begin();
            Vector<WideString>::iterator itResult = resultStrings.begin();
            while (it != strings.end() && itResult != resultStrings.end())
            {
                if (*it != *itResult)
                {
                    testSuccess = false;
                    break;
                }
                ++it;
                ++itResult;
            }
        }

        TEST_VERIFY(testSuccess);
    }

    DAVA_TEST (CleanLineTest)
    {
        WideString test1 = StringUtils::RemoveNonPrintable(L"THIS SOFTWARE\u00A0IS PROV\u200BIDED BY\n THE DAVA CONS\u200BULTING\n LLC");
        WideString test2 = StringUtils::RemoveNonPrintable(L"THIS\tSOFTWARE IS\tPROVIDED BY\nTHE DAVA CONSULTING\nLLC");
        WideString test3 = StringUtils::RemoveNonPrintable(L"THIS\tSOFTWARE IS\tPROVIDED BY\nTHE DAVA CONSULTING\nLLC", 1);
        WideString test4 = StringUtils::RemoveNonPrintable(L"THIS\tSOFTWARE IS\tPROVIDED BY\nTHE DAVA CONSULTING\nLLC", 4);

        WideString out1 = L"THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING LLC";
        WideString out2 = L"THIS\tSOFTWARE IS\tPROVIDED BYTHE DAVA CONSULTINGLLC";
        WideString out3 = L"THIS SOFTWARE IS PROVIDED BYTHE DAVA CONSULTINGLLC";
        WideString out4 = L"THIS    SOFTWARE IS    PROVIDED BYTHE DAVA CONSULTINGLLC";

        TEST_VERIFY(test1 == out1);
        TEST_VERIFY(test2 == out2);
        TEST_VERIFY(test3 == out3);
        TEST_VERIFY(test4 == out4);
    }

    DAVA_TEST (TestFitting)
    {
        staticText->SetMultiline(false);

        for (const auto& data : testData)
        {
            staticText->SetFittingOption(data.fitting);
            staticText->SetTextAlign(data.align);
            staticText->SetText(TEST_DATA);
            const WideString& result = staticText->GetVisualText();
            TEST_VERIFY(result == data.result);
        }
    }
};
