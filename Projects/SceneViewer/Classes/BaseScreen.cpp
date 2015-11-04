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


#include "BaseScreen.h"

int32 BaseScreen::screensCount = 0;

BaseScreen::BaseScreen()
    : UIScreen()
    , font(nullptr)
    , screenID(screensCount++)
{
    UIScreenManager::Instance()->RegisterScreen(screenID, this);
}

bool BaseScreen::SystemInput(UIEvent *currentInput)
{
    if ((currentInput->tid == DVKEY_BACK) && (UIEvent::Phase::KEY_DOWN == currentInput->phase))
    {
        SetPreviousScreen();
    }
    else
    {
        return UIScreen::SystemInput(currentInput);
    }
    return true;
}

void BaseScreen::SystemScreenSizeDidChanged(const Rect &newFullScreenSize)
{
    UnloadResources();
    LoadResources();
}

void BaseScreen::LoadResources()
{
	GetBackground()->SetColor(Color(0.f, 0.f, 0.f, 1.f));

    DVASSERT(font == NULL);
    font = FTFont::Create("~res:/Fonts/korinna.ttf");
    font->SetSize(20.f);
}

void BaseScreen::UnloadResources()
{
    SafeRelease(font);
	RemoveAllControls();
}

UIButton * BaseScreen::CreateButton(const Rect &rect, const WideString & text)
{
    DVASSERT(font);
    
    UIButton *button = new UIButton(rect);
    button->SetStateText(UIControl::STATE_NORMAL, text);
    button->SetStateTextAlign(UIControl::STATE_NORMAL, ALIGN_HCENTER | ALIGN_VCENTER);
    button->SetStateFont(UIControl::STATE_NORMAL, font);
    button->SetStateFontColor(UIControl::STATE_NORMAL, Color::White);
    button->SetStateFontColor(UIControl::STATE_PRESSED_INSIDE, Color(0.7f, 0.7f, 0.7f, 1.f));
    
    return button;
}

void BaseScreen::SetPreviousScreen() const
{
    if(screenID)
    {
        UIScreenManager::Instance()->SetScreen(screenID - 1);
    }
}

void BaseScreen::SetNextScreen() const
{
    if(screenID < screensCount - 1)
    {
        UIScreenManager::Instance()->SetScreen(screenID + 1);
    }
}


