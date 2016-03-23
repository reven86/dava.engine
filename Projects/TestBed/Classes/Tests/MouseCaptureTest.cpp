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


#include "Tests/MouseCaptureTest.h"
#include "Input/InputSystem.h"

using namespace DAVA;

MouseCaptureTest::MouseCaptureTest()
    : BaseScreen("MouseCaptureTest")
{
}

void MouseCaptureTest::LoadResources()
{
    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(17);
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    float32 startX = 0.f, startY = 0.f, w = 0.f, h = 0.f;
    w = screenSize.dx * 0.5f;
    h = screenSize.dy * 0.05f;
    startX = screenSize.dx * 0.5f - w * 0.5f;
    startY = h;
    // button mouse
    mouseCaptureTest = new UIButton(Rect(startX, startY, w, h));
    mouseCaptureTest->SetStateFont(0xFF, font);
    mouseCaptureTest->SetDebugDraw(true);
    mouseCaptureTest->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
    mouseCaptureTest->SetStateText(0xFF, L"Capture");
    mouseCaptureTest->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MouseCaptureTest::Capture));
    AddControl(mouseCaptureTest);

    BaseScreen::LoadResources();
    //TODO: Initialize resources here
}

void MouseCaptureTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    RemoveAllControls();

    SafeRelease(mouseCaptureTest);
}

void MouseCaptureTest::Capture(BaseObject* obj, void* datпппa, void* callerData)
{
    InputSystem::Instance()->SetMouseCaptureMode(InputSystem::eMouseCaptureMode::PINING);
}

bool MouseCaptureTest::SystemInput(UIEvent* currentInput)
{
    if ((InputSystem::Instance()->GetMouseCaptureMode() == InputSystem::eMouseCaptureMode::PINING)
        && (currentInput->key == Key::ENTER))
    {
        InputSystem::Instance()->SetMouseCaptureMode(InputSystem::eMouseCaptureMode::OFF);
    }

    return BaseScreen::SystemInput(currentInput);
}
