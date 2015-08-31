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
#include "TestChooserScreen.h"


TestChooserScreen::TestChooserScreen()
    :   testForRun(nullptr)
{
}

void TestChooserScreen::LoadResources()
{
    BaseScreen::LoadResources();
    
    CreateChooserUI();
}

void TestChooserScreen::OnFinish()
{
    DVASSERT(nullptr != testForRun);
}

bool TestChooserScreen::IsFinished() const
{
    return nullptr != testForRun;
}

void TestChooserScreen::OnButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    UIButton* button = static_cast<UIButton*>(obj);
    const String& testName = button->GetName();
    
    for(auto *test : testChain)
    {
        if (testName == test->GetSceneName())
        {
            testForRun = test;
        }
    }
    
    DVASSERT(nullptr != testForRun);
}

void TestChooserScreen::CreateChooserUI()
{
    ScopedPtr<FTFont> chooserFont(FTFont::Create("~res:/Fonts/korinna.ttf"));
    uint32 testNumber = 0;
    
    for (auto *test : testChain)
    {
        ScopedPtr<UIButton> button(new UIButton());
        button->SetName(test->GetSceneName());

        button->SetPosition(Vector2(40.0f, 40.0f + testNumber * 100));
        button->SetSize(Vector2(200.0f, 70.0f));
        button->SetStateFont(UIControl::STATE_NORMAL, chooserFont);
        button->SetStateText(UIButton::STATE_NORMAL, UTF8Utils::EncodeToWideString(test->GetSceneName()));
        button->SetStateTextAlign(UIButton::STATE_NORMAL, ALIGN_HCENTER | ALIGN_VCENTER);

        button->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
        button->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.5f, 0.6f, 0.5f, 0.5f));
        button->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
        button->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.75f, 0.85f, 0.75f, 0.5f));
        button->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
        button->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));

        button->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
        button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TestChooserScreen::OnButtonPressed));

        AddControl(button);

		testNumber++;
	}
}