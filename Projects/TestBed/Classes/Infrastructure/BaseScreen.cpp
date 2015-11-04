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



#include "Infrastructure/BaseScreen.h"
#include "Infrastructure/GameCore.h"

using namespace DAVA;

int32 BaseScreen::globalScreenId = 1;

BaseScreen::BaseScreen(const String & _screenName, int32 skipBeforeTests)
    : UIScreen()
    , currentScreenId(globalScreenId++)
    , exitButton(nullptr)
{
    SetName(_screenName);
    
    GameCore::Instance()->RegisterScreen(this);
}

BaseScreen::BaseScreen()
    : UIScreen()
    , currentScreenId(globalScreenId++)
{
    SetName("BaseScreen");
    GameCore::Instance()->RegisterScreen(this);
}

void BaseScreen::SystemScreenSizeDidChanged(const Rect &newFullScreenSize)
{
    UIScreen::SystemScreenSizeDidChanged(newFullScreenSize);
    UnloadResources();
    LoadResources();
}

bool BaseScreen::SystemInput(UIEvent *currentInput)
{
    if ((currentInput->tid == DVKEY_BACK) && (currentInput->phase == UIEvent::Phase::KEY_DOWN))
    {
        OnExitButton(nullptr, nullptr, nullptr);
    }
    else
    {
        return UIScreen::SystemInput(currentInput);
    }
    return true;
}

void BaseScreen::LoadResources()
{
    ScopedPtr<FTFont> font (FTFont::Create("~res:/Fonts/korinna.ttf"));

    font->SetSize(30);

    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    exitButton = new UIButton(Rect(static_cast<DAVA::float32>(screenSize.dx-300), static_cast<DAVA::float32>(screenSize.dy-30), 300.0, 30.0));
    exitButton->SetStateFont(0xFF, font);
    exitButton->SetStateFontColor(0xFF, Color::White);
    exitButton->SetStateText(0xFF, L"Exit From Screen");

    exitButton->SetDebugDraw(true);
    exitButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &BaseScreen::OnExitButton));
    AddControl(exitButton);
}

void BaseScreen::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(exitButton);
    
    UIScreen::UnloadResources();
}

void BaseScreen::OnExitButton(BaseObject *obj, void *data, void *callerData)
{
    GameCore::Instance()->ShowStartScreen();
}
