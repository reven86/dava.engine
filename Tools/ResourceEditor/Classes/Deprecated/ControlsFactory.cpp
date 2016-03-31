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


#include "ControlsFactory.h"
#include "Qt/Settings/SettingsManager.h"

Font* ControlsFactory::font12 = NULL;
Font* ControlsFactory::font20 = NULL;

UIButton* ControlsFactory::CreateButton(const Rect& rect, const WideString& buttonText, bool designers)
{
    UIButton* btn = new UIButton(rect);
    CustomizeButton(btn, buttonText, designers);
    return btn;
}

void ControlsFactory::CustomizeButton(UIButton* btn, const WideString& buttonText, bool designers)
{
    Font* font = GetFont12();

    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);

    if (designers)
    {
        btn->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(97.f / 255.f, 69.f / 255.f, 68.f / 255.f, 1.f));
    }
    else
    {
        btn->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    }
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    btn->GetStateBackground(UIControl::STATE_SELECTED)->SetColor(Color(0.0f, 0.0f, 1.0f, 0.2f));

    btn->SetStateFont(UIControl::STATE_PRESSED_INSIDE, font);
    btn->SetStateFont(UIControl::STATE_DISABLED, font);
    btn->SetStateFont(UIControl::STATE_NORMAL, font);
    btn->SetStateFont(UIControl::STATE_SELECTED, font);

    btn->SetStateText(UIControl::STATE_PRESSED_INSIDE, buttonText);
    btn->SetStateText(UIControl::STATE_DISABLED, buttonText);
    btn->SetStateText(UIControl::STATE_NORMAL, buttonText);
    btn->SetStateText(UIControl::STATE_SELECTED, buttonText);

    AddBorder(btn);
}

Font* ControlsFactory::GetFont12()
{
    if (!font12)
    {
        font12 = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
        font12->SetSize(12);
    }
    return font12;
}

Font* ControlsFactory::GetFont20()
{
    if (!font20)
    {
        font20 = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
        font20->SetSize(20);
    }
    return font12;
}

Color ControlsFactory::GetColorError()
{
    return Color(1.0f, 0.0f, 0.0f, 0.8f);
}

UIControl* ControlsFactory::CreateLine(const Rect& rect, Color color)
{
    UIControl* lineControl = new UIControl(rect);
    lineControl->GetBackground()->color = color;
    lineControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    return lineControl;
}

void ControlsFactory::CustomizeDialogFreeSpace(UIControl* c)
{
    c->GetBackground()->color = Color(0.0f, 0.0f, 0.0f, 0.3f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeDialog(UIControl* c)
{
    c->GetBackground()->color = Color(0.0f, 0.0f, 0.0f, 0.5f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::AddBorder(DAVA::UIControl* c)
{
    Rect fullRect = c->GetRect();

    Color lineColor(1.f, 1.f, 1.f, 0.5f);

    UIControl* leftLine = c->FindByName("LeftLine", false);
    if (!leftLine)
    {
        leftLine = ControlsFactory::CreateLine(Rect(0, 1, 1, fullRect.dy - 2), lineColor);
        leftLine->SetName("LeftLine");
        c->AddControl(leftLine);
        SafeRelease(leftLine);
    }

    UIControl* rightLine = c->FindByName("RightLine", false);
    if (!rightLine)
    {
        rightLine = ControlsFactory::CreateLine(Rect(fullRect.dx - 1, 1, 1, fullRect.dy - 2), lineColor);
        rightLine->SetName("RightLine");
        c->AddControl(rightLine);
        SafeRelease(rightLine);
    }

    UIControl* topLine = c->FindByName("TopLine", false);
    if (!topLine)
    {
        topLine = ControlsFactory::CreateLine(Rect(0, 0, fullRect.dx, 1), lineColor);
        topLine->SetName("TopLine");
        c->AddControl(topLine);
        SafeRelease(topLine);
    }

    UIControl* bottomtLine = c->FindByName("BottomLine", false);
    if (!bottomtLine)
    {
        bottomtLine = ControlsFactory::CreateLine(Rect(0, fullRect.dy - 1, fullRect.dx, 1), lineColor);
        bottomtLine->SetName("BottomLine");
        c->AddControl(bottomtLine);
        SafeRelease(bottomtLine);
    }
}

void ControlsFactory::ReleaseFonts()
{
    SafeRelease(font12);
    SafeRelease(font20);
}
