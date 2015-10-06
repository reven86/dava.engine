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

#include "Tests/KeyboardTest.h"

using namespace DAVA;

class CustomText : public UIStaticText
{
public:
    CustomText(const Rect& rect)
        : UIStaticText(rect)
    {
        SetInputEnabled(true);
    }

    bool SystemInput(UIEvent* currentInput) override
    {
        if (currentInput->phase >= UIEvent::PHASE_KEYCHAR &&
            currentInput->phase <= UIEvent::PHASE_KEY_DOWN_REPEAT)
        {
            numEvents++;
        }
        else
        {
            return false;
        }
        switch (currentInput->phase)
        {
        case UIEvent::PHASE_KEYCHAR:
            ++numChar;
            lastChar = currentInput->keyChar;
            break;
        case UIEvent::PHASE_KEYCHAR_REPEAT:
            ++numCharRepeat;
            break;
        case UIEvent::PHASE_KEY_DOWN:
            ++numKeyDown;
            break;
        case UIEvent::PHASE_KEY_DOWN_REPEAT:
            ++numKeyDownRepeat;
            break;
        case UIEvent::PHASE_KEY_UP:
            ++numKeyUp;
            break;
        case UIEvent::PHASE_WHEEL:
            //currentText += L"wheel ";
            break;
        };

        std::wstringstream currentText;
        currentText << L"events: " << numEvents << L"\n"
                    << L"numKeyDown: " << numKeyDown << L"\n"
                    << L"numKeyUp: " << numKeyUp << L"\n"
                    << L"numKeyDownRepeat: " << numKeyDownRepeat << L"\n"
                    << L"numChar: " << numChar << L"\n"
                    << L"numCharRepeat: " << numCharRepeat << L"\n"
                    << L"lastChar: \'" << lastChar << L"\'";

        SetText(currentText.str());
        return true;
    }

private:
    uint32 numEvents = 0;
    uint32 numKeyDown = 0;
    uint32 numKeyUp = 0;
    uint32 numKeyDownRepeat = 0;
    uint32 numChar = 0;
    uint32 numCharRepeat = 0;
    wchar_t lastChar = L'\0';
};

CustomText* customText = nullptr;

KeyboardTest::KeyboardTest()
    : BaseScreen("KeyboardTest")
{
}

void KeyboardTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    previewText = new UIStaticText(Rect(20, 30, 400, 200));
    previewText->SetFont(font);
    previewText->SetTextColor(Color::White);
    previewText->SetText(L"Press (Hold) and Unpress keys");
    previewText->SetDebugDraw(true);
    previewText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(previewText);

    customText = new CustomText(Rect(20, 230, 200, 400));
    customText->SetDebugDraw(true);
    customText->SetTextColor(Color::White);
    customText->SetFont(font);
    customText->SetFocusEnabled(true);
    customText->SetAlign(ALIGN_LEFT | ALIGN_TOP);
    customText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    //customText->SetHCenterAlign(ALIGN_LEFT);
    customText->SetMultiline(true);
    customText->SetMultilineType(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL);
    AddControl(customText);

    UIControlSystem::Instance()->SetFocusedControl(customText, true);
}

void KeyboardTest::UnloadResources()
{
    SafeRelease(customText);
    SafeRelease(previewText);

    BaseScreen::UnloadResources();
}
