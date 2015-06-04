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

#include "SpeedLoadImagesTest.h"

#include "Platform/SystemTimer.h"
#include "Render/Image/ImageSystem.h"
#include "Utils/UTF8Utils.h"

SpeedLoadImagesTest::SpeedLoadImagesTest()
    : BaseScreen("SpeedLoadImagesTest")
    , testPngBtn(nullptr)
    , resultText(nullptr)
{
}

SpeedLoadImagesTest::~SpeedLoadImagesTest()
{
}

void SpeedLoadImagesTest::LoadResources()
{
    BaseScreen::LoadResources();
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(30);

    testPngBtn = new UIButton(Rect(10, 10, 450, 60));
    testPngBtn->SetStateFont(0xFF, font);
    testPngBtn->SetStateFontColor(0xFF, Color::White);
    testPngBtn->SetStateText(0xFF, L"Test PNG");

    testPngBtn->SetDebugDraw(true);
    testPngBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SpeedLoadImagesTest::OnTestPng));
    AddControl(testPngBtn);

    testJpgBtn = new UIButton(Rect(500, 10, 450, 60));
    testJpgBtn->SetStateFont(0xFF, font);
    testJpgBtn->SetStateFontColor(0xFF, Color::White);
    testJpgBtn->SetStateText(0xFF, L"Test JPG");

    testJpgBtn->SetDebugDraw(true);
    testJpgBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SpeedLoadImagesTest::OnTestJpg));
    AddControl(testJpgBtn);

    testTgaBtn = new UIButton(Rect(10, 100, 450, 60));
    testTgaBtn->SetStateFont(0xFF, font);
    testTgaBtn->SetStateFontColor(0xFF, Color::White);
    testTgaBtn->SetStateText(0xFF, L"Test TGA");

    testTgaBtn->SetDebugDraw(true);
    testTgaBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SpeedLoadImagesTest::OnTestTga));
    AddControl(testTgaBtn);

    testWebPBtn = new UIButton(Rect(500, 100, 450, 60));
    testWebPBtn->SetStateFont(0xFF, font);
    testWebPBtn->SetStateFontColor(0xFF, Color::White);
    testWebPBtn->SetStateText(0xFF, L"Test WebP");

    testWebPBtn->SetDebugDraw(true);
    testWebPBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SpeedLoadImagesTest::OnTestWebP));
    AddControl(testWebPBtn);

    resultText = new UIStaticText(Rect(10, 190, 990, 1400));
    resultText->SetFont(font);
    resultText->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    resultText->GetBackground()->SetColor(Color(0.0, 0.0, 0.0, 1.0));
    resultText->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    resultText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    resultText->SetMultiline(true);
    AddControl(resultText);

    SafeRelease(font);
}

void SpeedLoadImagesTest::UnloadResources()
{
    RemoveAllControls();
    SafeRelease(testPngBtn);
    SafeRelease(testJpgBtn);
    SafeRelease(testTgaBtn);
    SafeRelease(testWebPBtn);

    SafeRelease(resultText);

    BaseScreen::UnloadResources();
}

void SpeedLoadImagesTest::OnTestPng(BaseObject *obj, void *data, void *callerData)
{
    String resultString;
    resultString.append("Results:");
    resultText->SetText(UTF8Utils::EncodeToWideString(resultString));

    Vector<String> qualities;
    qualities.push_back("100");
    TestAndDisplayFormat("png", qualities);
}

void SpeedLoadImagesTest::OnTestJpg(BaseObject *obj, void *data, void *callerData)
{
    String resultString;
    resultString.append("Results:");
    resultText->SetText(UTF8Utils::EncodeToWideString(resultString));

    Vector<String> qualities;
    qualities.push_back("100");
    TestAndDisplayFormat("jpeg", qualities);
}

void SpeedLoadImagesTest::OnTestTga(BaseObject *obj, void *data, void *callerData)
{
    String resultString;
    resultString.append("Results:");
    resultText->SetText(UTF8Utils::EncodeToWideString(resultString));

    Vector<String> qualities;
    qualities.push_back("100");
    TestAndDisplayFormat("tga", qualities);
}

void SpeedLoadImagesTest::OnTestWebP(BaseObject *obj, void *data, void *callerData)
{
    String resultString;
    resultString.append("Results:");
    resultText->SetText(UTF8Utils::EncodeToWideString(resultString));

    Vector<String> qualities;
    qualities.push_back("60");
    qualities.push_back("70");
    qualities.push_back("80");
    qualities.push_back("90");
    qualities.push_back("95");
    qualities.push_back("100");
    TestAndDisplayFormat("webp", qualities);
}

void SpeedLoadImagesTest::TestAndDisplayFormat(String extension, Vector<String> qualities)
{
    String resultString("\n");
    Vector<FilePath> paths = CreatePaths(extension, qualities);
    for (auto path : paths)
    {
        float loadTime = GetLoadTime(path);
        auto fileName = path.GetFilename();
        resultString.append(Format("%s: %s - %f\n", extension.c_str(), fileName.c_str(), loadTime));
    }

    if (paths.empty())
    {
        resultString.append(Format("Files *.%s not found.\n", extension.c_str()));
    }

    WideString resultWideString = resultText->GetText();
    resultWideString.append(UTF8Utils::EncodeToWideString(resultString));
    resultText->SetText(resultWideString);
}

Vector<FilePath> SpeedLoadImagesTest::CreatePaths(String extension, Vector<String> qualities)
{
    Vector<FilePath> paths;

    Vector<String> fileNames;
    fileNames.push_back("_rgb888_512");
    fileNames.push_back("_rgb888_1024");
    fileNames.push_back("_rgb888_2048");
    fileNames.push_back("_rgba8888_512");
    fileNames.push_back("_rgba8888_1024");
    fileNames.push_back("_rgba8888_2048");

    for (auto fileName : fileNames)
    {
        for (auto quality : qualities)
        {
            FilePath inpath(Format("~res:/TestData/SpeedLoadImagesTest/%s%s_quality%s.%s",
                                   extension.c_str(),
                                   fileName.c_str(),
                                   quality.c_str(),
                                   extension.c_str()));

            File *infile = File::Create(inpath, File::OPEN | File::READ);
            if (infile != nullptr)
            //if (inpath.Exists())
            {
                paths.push_back(inpath);
                infile->Release();
            }
        }
    }

    return paths;
}

float SpeedLoadImagesTest::GetLoadTime(FilePath path)
{
    File *infile = File::Create(path, File::OPEN | File::READ);
    Vector<Image *> imageSet;

    auto number = 5;
    float allTime = 0;
    for (auto i = 0; i < number; ++i)
    {
        uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
        ImageSystem::Instance()->Load(infile, imageSet);
        uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
        allTime += finishTime - startTime;

        for (auto image : imageSet)
        {
            image->Release();
        }
        imageSet.clear();
    }

    infile->Release();

    return allTime / number;
}
