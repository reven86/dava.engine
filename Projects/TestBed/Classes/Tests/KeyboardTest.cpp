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
        UIStaticText::SetInputEnabled(true);
    }

    bool SystemInput(UIEvent* currentInput) override
    {
        if (currentInput->phase >= UIEvent::PHASE_CHAR &&
            currentInput->phase <= UIEvent::PHASE_KEY_DOWN_REPEAT)
        {
            ++numKeyboardEvents;
        }
        else if (currentInput->phase >= UIEvent::PHASE_BEGAN ||
                 currentInput->phase <= UIEvent::PHASE_CANCELLED)
        {
            ++numMouseEvents;
        }
        switch (currentInput->phase)
        {
        case UIEvent::PHASE_BEGAN: //!<Screen touch or mouse button press is began.
            ++numMouseDown;
            lastMouseX = currentInput->point.x;
            lastMouseY = currentInput->point.y;
            lastMouseKey = currentInput->tid;
            break;
        case UIEvent::PHASE_DRAG: //!<User moves mouse with presset button or finger over the screen.
            ++numDrag;
            lastMouseX = currentInput->point.x;
            lastMouseY = currentInput->point.y;
            break;
        case UIEvent::PHASE_ENDED: //!<Screen touch or mouse button press is ended.
            ++numMouseUp;
            lastMouseX = currentInput->point.x;
            lastMouseY = currentInput->point.y;
            lastMouseKey = currentInput->tid;
            break;
        case UIEvent::PHASE_MOVE: //!<Mouse move event. Mouse moves without pressing any buttons. Works only with mouse controller.
            ++numMouseMove;
            lastMouseX = currentInput->point.x;
            lastMouseY = currentInput->point.y;
            lastMouseKey = currentInput->tid;
            break;
        case UIEvent::PHASE_WHEEL: //!<Mouse wheel event. MacOS & Win32 only
            ++numMouseWheel;
            lastWheel = currentInput->physPoint.y;
            break;
        case UIEvent::PHASE_CANCELLED: //!<Event was cancelled by the platform or by the control system for the some reason.
            ++numMouseCancel;
            break;
        case UIEvent::PHASE_CHAR:
            ++numChar;
            lastChar = currentInput->keyChar;
            break;
        case UIEvent::PHASE_CHAR_REPEAT:
            ++numCharRepeat;
            break;
        case UIEvent::PHASE_KEY_DOWN:
            ++numKeyDown;
            lastKey = currentInput->tid;
            break;
        case UIEvent::PHASE_KEY_DOWN_REPEAT:
            ++numKeyDownRepeat;
            lastKey = currentInput->tid;
            OutputDebugStringA("PHASE_KEY_DOWN_REPEAT\n");
            break;
        case UIEvent::PHASE_KEY_UP:
            ++numKeyUp;
            lastKey = currentInput->tid;
            break;
        };

        std::wstringstream currentText;
        currentText << L"Keyboards: " << numKeyboardEvents << L"\n"
                    << L"KD: " << numKeyDown << L"\n"
                    << L"KU: " << numKeyUp << L"\n"
                    << L"KDR: " << numKeyDownRepeat << L"\n"
                    << L"Ch: " << numChar << L"\n"
                    << L"ChR: " << numCharRepeat << L"\n"
                    << L"ch: \'" << lastChar << L"\'\n"
                    << L"k: " << lastKey << L"\n"
                    << L"Mouse: " << numMouseEvents << L"\n"
                    << L"Drg: " << numDrag << L"\n"
                    << L"Mv: " << numMouseMove << L"\n"
                    << L"Dn: " << numMouseDown << L"\n"
                    << L"Up: " << numMouseUp << L"\n"
                    << L"Whl: " << numMouseWheel << L"\n"
                    << L"Cncl: " << numMouseCancel << L"\n"
                    << L"Key: " << lastMouseKey << L"\n"
                    << L"X: " << lastMouseX << L"\n"
                    << L"Y: " << lastMouseY << L"\n"
                    << L"Whl: " << lastWheel;

        SetText(currentText.str());

        return true;
    }

    void ResetCounters()
    {
        numKeyboardEvents = 0;
        numKeyDown = 0;
        numKeyUp = 0;
        numKeyDownRepeat = 0;
        numChar = 0;
        numCharRepeat = 0;
        lastChar = L'\0';
        lastKey = 0;

        numMouseEvents = 0;
        numDrag = 0;
        numMouseMove = 0;
        numMouseDown = 0;
        numMouseUp = 0;
        numMouseWheel = 0;
        numMouseCancel = 0;
        lastMouseKey = L'\0';
        lastMouseX = 0;
        lastMouseY = 0;
        lastWheel = 0.f;
    }

private:
    uint32 numKeyboardEvents = 0;
    uint32 numKeyDown = 0;
    uint32 numKeyUp = 0;
    uint32 numKeyDownRepeat = 0;
    uint32 numChar = 0;
    uint32 numCharRepeat = 0;
    wchar_t lastChar = L'\0';
    uint32 lastKey = 0;

    uint32 numMouseEvents = 0;
    uint32 numDrag = 0;
    uint32 numMouseMove = 0;
    uint32 numMouseDown = 0;
    uint32 numMouseUp = 0;
    uint32 numMouseWheel = 0;
    uint32 numMouseCancel = 0;
    wchar_t lastMouseKey = L'\0';
    int32 lastMouseX = 0;
    int32 lastMouseY = 0;
    float32 lastWheel = 0.f;
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
    customText->SetMultiline(true);
    customText->SetMultilineType(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL);
    AddControl(customText);

    resetButton = new UIButton(Rect(420, 30, 50, 30));
    resetButton->SetDebugDraw(true);
    resetButton->SetStateFont(0xFF, font);
    resetButton->SetStateFontColor(0xFF, Color::White);
    resetButton->SetStateText(0xFF, L"Reset");
    resetButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &KeyboardTest::OnResetClick));
    AddControl(resetButton);

    UIControlSystem::Instance()->SetFocusedControl(customText, true);
}

void KeyboardTest::UnloadResources()
{
    SafeRelease(customText);
    SafeRelease(previewText);
    SafeRelease(resetButton);

    BaseScreen::UnloadResources();
}

void KeyboardTest::OnResetClick(DAVA::BaseObject* sender, void* data, void* callerData)
{
    if (customText)
    {
        customText->ResetCounters();
        customText->SetText(L"");
    }
}
