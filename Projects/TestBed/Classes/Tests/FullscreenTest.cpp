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


#include "Tests/FullscreenTest.h"

using namespace DAVA;

FullscreenTest::FullscreenTest()
    : BaseScreen("FullscreenTest")
{
}

void FullscreenTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    currentModeText = new UIStaticText(Rect(310, 10, 300, 20));
    currentModeText->SetFont(font);
    currentModeText->SetTextColor(Color::White);
    AddControl(currentModeText);

    ScopedPtr<UIButton> btn(new UIButton(Rect(10, 40, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Windowed");
    btn->SetDebugDraw(true);
    btn->SetTag(0);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 70, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Fullsreen");
    btn->SetDebugDraw(true);
    btn->SetTag(1);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 100, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Windowed fullscreen (borderless)");
    btn->SetDebugDraw(true);
    btn->SetTag(2);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 10, 300, 20)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Refresh status");
    btn->SetDebugDraw(true);
    btn->SetTag(99);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnSelectModeClick));
    AddControl(btn);

    btn.reset(new UIButton(Rect(10, 150, 145, 30)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mul +0.1");
    btn->SetDebugDraw(true);
    btn->SetTag(99);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnMulUp));
    AddControl(btn);

    btn.reset(new UIButton(Rect(155, 150, 145, 30)));
    btn->SetStateFont(0xFF, font);
    btn->SetStateText(0xFF, L"Mul -0.1");
    btn->SetDebugDraw(true);
    btn->SetTag(99);
    btn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &FullscreenTest::OnMulDown));
    AddControl(btn);

    currentScaleText = new UIStaticText(Rect(310, 150, 300, 30));
    currentScaleText->SetFont(font);
    currentScaleText->SetTextColor(Color::White);
    currentScaleText->SetText(Format(L"%f", Core::Instance()->GetScreenScaleMultiplier()));

    AddControl(currentScaleText);

    GetBackground()->SetColor(Color::White);

    UpdateMode();
}

void FullscreenTest::UnloadResources()
{
    SafeRelease(currentModeText);

    BaseScreen::UnloadResources();
}

void FullscreenTest::OnSelectModeClick(BaseObject* sender, void* data, void* callerData)
{
    UIButton* btn = static_cast<UIButton*>(sender);
    switch (btn->GetTag())
    {
    case 0:
        Core::Instance()->SetScreenMode(Core::eScreenMode::WINDOWED);
        break;
    case 1:
        Core::Instance()->SetScreenMode(Core::eScreenMode::FULLSCREEN);
        break;
    case 2:
        Core::Instance()->SetScreenMode(Core::eScreenMode::WINDOWED_FULLSCREEN);
        break;
    case 99:
        UpdateMode();
        break;
    }
}

void FullscreenTest::OnMulUp(BaseObject* sender, void* data, void* callerData)
{
    float32 mul = Core::Instance()->GetScreenScaleMultiplier();
    if (mul < 2.0)
    {
        mul += 0.1;
    }

    Core::Instance()->SetScreenScaleMultiplier(mul);

    currentScaleText->SetText(Format(L"%f", mul));
}

void FullscreenTest::OnMulDown(BaseObject* sender, void* data, void* callerData)
{
    float32 mul = Core::Instance()->GetScreenScaleMultiplier();
    if (mul > 0.2)
    {
        mul -= 0.1;
    }

    Core::Instance()->SetScreenScaleMultiplier(mul);

    currentScaleText->SetText(Format(L"%f", mul));
}

void FullscreenTest::UpdateMode()
{
    switch (Core::Instance()->GetScreenMode())
    {
    case Core::eScreenMode::WINDOWED:
        currentModeText->SetText(L"Windowed");
        break;
    case Core::eScreenMode::WINDOWED_FULLSCREEN:
        currentModeText->SetText(L"Windowed fullscreen");
        break;
    case Core::eScreenMode::FULLSCREEN:
        currentModeText->SetText(L"Fullscreen");
        break;
    default:
        currentModeText->SetText(L"Unknown");
        break;
    }
}
