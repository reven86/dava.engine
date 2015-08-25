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

using namespace DAVA;

SpeedLoadImagesTest::SpeedLoadImagesTest()
    : BaseScreen("SpeedLoadImagesTest")
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

    auto CreateButton = [font, this](const Rect &r, const WideString &str, const Message &m)
    {
        UIButton *button = new UIButton(r);
        button->SetStateFont(0xFF, font);
        button->SetStateFontColor(0xFF, Color::White);
        button->SetStateText(0xFF, str);
        button->SetDebugDraw(true);
        button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, m);
        AddControl(button);
        SafeRelease(button);
    };

    CreateButton(Rect(10, 10, 450, 60), L"Test PNG", Message(this, &SpeedLoadImagesTest::OnTestPng));
    CreateButton(Rect(500, 10, 450, 60), L"Test JPG", Message(this, &SpeedLoadImagesTest::OnTestJpg));
    CreateButton(Rect(10, 100, 450, 60), L"Test TGA", Message(this, &SpeedLoadImagesTest::OnTestTga));
    CreateButton(Rect(500, 100, 450, 60), L"Test WebP", Message(this, &SpeedLoadImagesTest::OnTestWebP));
    CreateButton(Rect(10, 190, 450, 60), L"Test PVR", Message(this, &SpeedLoadImagesTest::OnTestPvr));

    resultText = new UIStaticText(Rect(10, 280, 700, 1400));
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
    TestAndDisplayFormat("jpg", qualities);
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

void SpeedLoadImagesTest::OnTestPvr(BaseObject *obj, void *data, void *callerData)
{
    String resultString;
    resultString.append("Results:");
    resultText->SetText(UTF8Utils::EncodeToWideString(resultString));

    Vector<String> qualities;
    qualities.push_back("100");
    TestAndDisplayFormat("pvr", qualities);
}

void SpeedLoadImagesTest::TestAndDisplayFormat(String extension, const Vector<String> &qualities)
{
    FileSystem::Instance()->CreateDirectory("~doc:/TestData/SpeedLoadImagesTest/", true);
    FilePath resultsPath(Format("~doc:/TestData/SpeedLoadImagesTest/results_%s.txt", extension.c_str()));
    ScopedPtr<File> resultsFile(File::Create(resultsPath, File::CREATE | File::WRITE)); 
    
    String resultString("\n");
    Vector<FilePath> paths;
    CreatePaths(extension, qualities, paths);
    if (paths.empty())
    {
        resultString.append(Format("Files *.%s not found.\n", extension.c_str()));
    }
    else
    {
        for (auto &path : paths)
        {
            uint64 loadTime = GetLoadTime(path);
            auto fileName = path.GetFilename();
            String str = Format("%s - %d\n", fileName.c_str(), loadTime);
            resultString.append(str);
            resultsFile->WriteLine(str);
        }
    }

    WideString resultWideString = resultText->GetText();
    resultWideString.append(UTF8Utils::EncodeToWideString(resultString));
    resultText->SetText(resultWideString);
}

void SpeedLoadImagesTest::CreatePaths(String extension, const Vector<String> &qualities, Vector<FilePath> &outPaths)
{
    Vector<String> fileNames;
    fileNames.push_back("_rgb888_512");
    fileNames.push_back("_rgb888_1024");
    fileNames.push_back("_rgb888_2048");
    fileNames.push_back("_rgba8888_512");
    fileNames.push_back("_rgba8888_1024");
    fileNames.push_back("_rgba8888_2048");
    fileNames.push_back("_pvr2_512");
    fileNames.push_back("_pvr2_1024");
    fileNames.push_back("_pvr2_2048");
    fileNames.push_back("_pvr4_512");
    fileNames.push_back("_pvr4_1024");
    fileNames.push_back("_pvr4_2048");

    for (auto &fileName : fileNames)
    {
        for (auto &quality : qualities)
        {
            FilePath inpath(Format("~res:/TestData/SpeedLoadImagesTest/%s%s_quality%s.%s",
                                   extension.c_str(),
                                   fileName.c_str(),
                                   quality.c_str(),
                                   extension.c_str()));

            if (inpath.Exists())
            {
                outPaths.push_back(inpath);
            }
        }
    }
}

uint64 SpeedLoadImagesTest::GetLoadTime(const FilePath &path)
{
    ScopedPtr<File> infile(File::Create(path, File::OPEN | File::READ));
    Vector<Image *> imageSet;

    auto number = 5;
    uint64 allTime = 0;
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

    return allTime / number;
}
