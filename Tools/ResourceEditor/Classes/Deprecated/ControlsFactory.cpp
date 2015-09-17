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

UIButton * ControlsFactory::CreateButton(Vector2 pos, const WideString &buttonText, bool designers)
{
    UIButton *btn = new UIButton(Rect(pos.x, pos.y, BUTTON_WIDTH, BUTTON_HEIGHT));
    CustomizeButton(btn, buttonText, designers);
    return btn;
}

UIButton * ControlsFactory::CreateButton(const Rect & rect, const WideString &buttonText, bool designers)
{
    UIButton *btn = new UIButton(rect);
    CustomizeButton(btn, buttonText, designers);
    return btn;
}

void ControlsFactory::CustomizeButton(UIButton *btn, const WideString &buttonText, bool designers)
{
    Font *font = GetFont12();
    
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    
    if(designers)
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

void ControlsFactory::CustomizeButtonExpandable(UIButton *btn)
{
    //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    Texture::SetDefaultGPU(GPU_ORIGIN);
    
    UIControl *expandable = new UIControl(Rect(btn->GetSize().dx - btn->GetSize().dy, 0, btn->GetSize().dy, btn->GetSize().dy));
    expandable->SetInputEnabled(false);
    expandable->SetSprite("~res:/Gfx/UI/arrowdown", 0);
    btn->AddControl(expandable);
    
    SafeRelease(expandable);
    
    Texture::SetDefaultGPU( static_cast<eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}


UIButton *ControlsFactory::CreateImageButton(const Rect & rect, const FilePath &imagePath)
{
    UIButton *btn = new UIButton(rect);
    CustomizeImageButton(btn, imagePath);
    return btn;
}

void ControlsFactory::CustomizeImageButton(UIButton *btn, const FilePath &imagePath)
{
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_SCALE_TO_RECT);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_SCALE_TO_RECT);
    btn->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_SCALE_TO_RECT);
    btn->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_SCALE_TO_RECT);

    
    btn->SetStateSprite(UIControl::STATE_PRESSED_INSIDE, imagePath);
    btn->SetStateSprite(UIControl::STATE_DISABLED, imagePath);
    btn->SetStateSprite(UIControl::STATE_NORMAL, imagePath);
    btn->SetStateSprite(UIControl::STATE_SELECTED, imagePath);
    
    btn->SetStateFrame(UIControl::STATE_PRESSED_INSIDE, 0);
    btn->SetStateFrame(UIControl::STATE_DISABLED, 0);
    btn->SetStateFrame(UIControl::STATE_NORMAL, 0);
    btn->SetStateFrame(UIControl::STATE_SELECTED, 0);
}



UIButton * ControlsFactory::CreateCloseWindowButton(const Rect & rect)
{
    UIButton *btn = new UIButton(rect);
    CustomizeCloseWindowButton(btn);
    return btn;
}

void ControlsFactory::CustomizeCloseWindowButton(UIButton *btn)
{
    Font *font = GetFont12();
    
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    
    btn->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(1.0f, 0.0f, 0.0f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.8f, 0.0f, 0.0f, 0.5f));
    
    btn->SetStateFont(UIControl::STATE_PRESSED_INSIDE, font);
    btn->SetStateFont(UIControl::STATE_DISABLED, font);
    btn->SetStateFont(UIControl::STATE_NORMAL, font);
    btn->SetStateFont(UIControl::STATE_SELECTED, font);
    
    btn->SetStateText(UIControl::STATE_PRESSED_INSIDE, L"X");
    btn->SetStateText(UIControl::STATE_DISABLED, L"X");
    btn->SetStateText(UIControl::STATE_NORMAL, L"X");
    btn->SetStateText(UIControl::STATE_SELECTED, L"X");

    AddBorder(btn);
}

Font * ControlsFactory::GetFont12()
{
	if (!font12) 
	{
		font12 = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
		font12->SetSize(12);
	}
	return font12;
}


Font * ControlsFactory::GetFont20()
{
	if (!font20) 
	{
		font20 = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
		font20->SetSize(20);
	}
	return font12;
}

Color ControlsFactory::GetColorLight()
{
    return Color(1.0f, 1.0f, 1.0f, 1.0f);
}

Color ControlsFactory::GetColorDark()
{
    return Color(0.0f, 0.0f, 0.0f, 1.0f);
}

Color ControlsFactory::GetColorError()
{
	return Color(1.0f, 0.0f, 0.0f, 0.8f);
}

void ControlsFactory::CustomizeScreenBack(UIControl *screen)
{
    screen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    screen->GetBackground()->SetColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
}

UIControl * ControlsFactory::CreateLine(const Rect & rect)
{
    return CreateLine(rect, Color(0.8f, 0.8f, 0.8f, 1.0f));
}

UIControl * ControlsFactory::CreateLine(const Rect & rect, Color color)
{
    UIControl * lineControl = new UIControl(rect); 
    lineControl->GetBackground()->color = color;
    lineControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    return lineControl;
}


void ControlsFactory::CusomizeBottomLevelControl(UIControl *c)
{
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
}

void ControlsFactory::CusomizeTransparentControl(UIControl *c, float32 transparentLevel)
{
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(Color(0.0f, 0.0f, 0.0f, transparentLevel));
}



void ControlsFactory::CusomizeTopLevelControl(UIControl *c)
{
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
}

void ControlsFactory::CusomizeListControl(UIControl *c)
{
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(Color(0.92f, 0.92f, 0.92f, 1.0f));
}

UIControl * ControlsFactory::CreatePanelControl(const Rect & rect, bool addBorder)
{
    UIControl *ctrl = new UIControl(rect);
    CustomizePanelControl(ctrl, addBorder);
    return ctrl;
}

void ControlsFactory::CustomizePanelControl(UIControl *c, bool addBorder)
{
    c->GetBackground()->color = Color(0.4f, 0.4f, 0.4f, 1.0f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    
    if(addBorder)
    {
        AddBorder(c);
    }
}

void ControlsFactory::CustomizeExpandButton(UIButton *btn)
{
    Color color(0.1f, 0.5f, 0.05f, 1.0f);
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_HOVER, UIControlBackground::DRAW_FILL);
    btn->GetStateBackground(UIControl::STATE_NORMAL)->color = color;
    btn->GetStateBackground(UIControl::STATE_HOVER)->color = color + 0.1f;
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->color = color + 0.3f;
}

void ControlsFactory::CustomizeListCell(UIListCell *c, const WideString &text, bool setLightFont)
{
    Font *font = GetFont12();
    
    c->SetStateFont(UIControl::STATE_NORMAL, font);
    c->SetStateFont(UIControl::STATE_SELECTED, font);

    c->SetStateText(UIControl::STATE_NORMAL, text);
    c->SetStateText(UIControl::STATE_SELECTED, text);

    
    c->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    c->GetStateBackground(UIControl::STATE_NORMAL)->color = Color(1.0f, 1.0f, 1.0f, 0.0f);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(1.0f, 0.8f, 0.8f, 1.0f);

    if(setLightFont)
    {
        c->GetStateTextControl(UIControl::STATE_NORMAL)->SetTextColor(Color::White);
        c->GetStateTextControl(UIControl::STATE_SELECTED)->SetTextColor(Color::White);
    }
    else
    {
        c->GetStateTextControl(UIControl::STATE_NORMAL)->SetTextColor(Color::Black);
        c->GetStateTextControl(UIControl::STATE_SELECTED)->SetTextColor(Color::Black);
    }
}

void ControlsFactory::CustomizeListCellAlternative(UIListCell *c, const WideString &text)
{
    Font *font = GetFont12();
    
    c->SetStateFont(UIControl::STATE_NORMAL, font);
    c->SetStateFont(UIControl::STATE_SELECTED, font);
    
    c->SetStateText(UIControl::STATE_NORMAL, text);
    c->SetStateText(UIControl::STATE_SELECTED, text);
    

    c->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    c->GetStateBackground(UIControl::STATE_NORMAL)->color = Color(0.4f, 0.8f, 0.4f, 1.0f);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(1.0f, 0.8f, 0.8f, 1.0f);
}


void ControlsFactory::CustomizeSceneGraphCell(UIHierarchyCell *c)
{
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(1.0f, 0.8f, 0.8f, 1.0f);
}

void ControlsFactory::CustomizeMenuPopupCell(UIListCell *c, const WideString &text)
{
    Font *font = GetFont12();
    
    c->SetStateFont(UIControl::STATE_NORMAL, font);
    c->SetStateFont(UIControl::STATE_SELECTED, font);

    c->SetStateText(UIControl::STATE_NORMAL, text);
    c->SetStateText(UIControl::STATE_SELECTED, text);
    
    c->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    
    c->GetStateBackground(UIControl::STATE_NORMAL)->color = Color(0.1f, 0.1f, 0.1f, 0.6f);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(0.6f, 0.6f, 0.6f, 0.6f);
    
    
    Rect rect = c->GetRect();
    rect.y = rect.dy - 1;
    rect.dy = 1;
    UIControl *line = CreateLine(rect);
    c->AddControl(line);
    SafeRelease(line);
}

void ControlsFactory::CustomizePropertyCell(DAVA::UIControl *c, bool isActivePart)
{
    if(isActivePart)
    {
        c->GetBackground()->color = Color(0.5f, 0.5f, 0.5f, 0.5f);
    }
    else
    {
        c->GetBackground()->color = Color(0.4f, 0.4f, 0.4f, 1.0f);
    }
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeEditablePropertyCell(UIControl *c)
{
    c->GetBackground()->color = Color(0.2f, 0.2f, 0.2f, 0.6f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeUneditablePropertyCell(UIControl *c)
{
    c->GetBackground()->color = Color(0.4f, 0.4f, 0.4f, 0.5f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizePropertySectionCell(UIControl *c)
{
    c->GetBackground()->color = Color(0.4f, 0.8f, 0.4f, 1.0f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizePropertySubsectionCell(UIControl *c)
{
    c->GetBackground()->color = Color(0.4f, 0.8f, 0.4f, 0.5f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizePropertyButtonCell(UIListCell *c)
{
    c->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    
    c->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    c->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    c->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    c->GetStateBackground(UIControl::STATE_SELECTED)->SetColor(Color(0.0f, 0.0f, 1.0f, 0.2f));
    
    Font *font = GetFont12();
    c->SetStateFont(UIControl::STATE_PRESSED_INSIDE, font);
    c->SetStateFont(UIControl::STATE_DISABLED, font);
    c->SetStateFont(UIControl::STATE_NORMAL, font);
    c->SetStateFont(UIControl::STATE_SELECTED, font);
}


void ControlsFactory::CustomizeDialogFreeSpace(UIControl *c)
{
    c->GetBackground()->color = Color(0.0f, 0.0f, 0.0f, 0.3f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeDialog(UIControl *c)
{
    c->GetBackground()->color = Color(0.0f, 0.0f, 0.0f, 0.5f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::SetScrollbar(DAVA::UIList *l)
{
    //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    Texture::SetDefaultGPU(GPU_ORIGIN);

    
    UIControl *c = l->FindByName("ScrollBar");
    if(c) return;
    
    Rect fr = l->GetRect();
    
    Sprite *scrollSpr = Sprite::Create("~res:/Gfx/UI/scroll");
    
    UIScrollBar *scrollBar = new UIScrollBar(Rect(fr.dx - scrollSpr->GetWidth(), 0, scrollSpr->GetWidth(), fr.dy), 
                                             UIScrollBar::ORIENTATION_VERTICAL);
    scrollBar->SetName("ScrollBar");
    
    scrollBar->GetSlider()->SetSprite(scrollSpr, 0);
    scrollBar->GetSlider()->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_VERTICAL);
    scrollBar->GetSlider()->GetBackground()->SetTopBottomStretchCap(10);
    
    scrollBar->SetDelegate(l);
    scrollBar->SetInputEnabled(false);
    l->AddControl(scrollBar);
    
    SafeRelease(scrollSpr);
    SafeRelease(scrollBar);
    
    Texture::SetDefaultGPU(static_cast<eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}

void ControlsFactory::RemoveScrollbar(UIList *l)
{
    UIControl *scrollBar = l->FindByName("ScrollBar");
    if(scrollBar)
    {
        scrollBar->GetParent()->RemoveControl(scrollBar);
    }
}


void ControlsFactory::SetScrollbar(DAVA::UIHierarchy *h)
{
    //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    Texture::SetDefaultGPU(GPU_ORIGIN);
    
    Rect fr = h->GetRect();
    
    Sprite *scrollSpr = Sprite::Create("~res:/Gfx/UI/scroll");
    
    UIScrollBar *scrollBar = new UIScrollBar(Rect(fr.dx - scrollSpr->GetWidth(), 0, scrollSpr->GetWidth(), fr.dy), 
                                             UIScrollBar::ORIENTATION_VERTICAL);
    
    scrollBar->GetSlider()->SetSprite(scrollSpr, 0);
    scrollBar->GetSlider()->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_VERTICAL);
    scrollBar->GetSlider()->GetBackground()->SetTopBottomStretchCap(10);
    
    scrollBar->SetDelegate(h);
    scrollBar->SetInputEnabled(false);
    h->AddControl(scrollBar);
    
    SafeRelease(scrollSpr);
    SafeRelease(scrollBar);
    
    Texture::SetDefaultGPU(static_cast<eGPUFamily>(SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32()));
}

void ControlsFactory::AddBorder(DAVA::UIControl *c)
{
    Rect fullRect = c->GetRect();
    
    Color lineColor(1.f, 1.f, 1.f, 0.5f);
    
    UIControl *leftLine = c->FindByName("LeftLine", false);
    if(!leftLine)
    {
        leftLine = ControlsFactory::CreateLine(Rect(0, 1, 1, fullRect.dy - 2), lineColor);
        leftLine->SetName("LeftLine");
        c->AddControl(leftLine);
        SafeRelease(leftLine);
    }

    UIControl *rightLine = c->FindByName("RightLine", false);
    if(!rightLine)
    {
        rightLine = ControlsFactory::CreateLine(Rect(fullRect.dx - 1, 1, 1, fullRect.dy - 2), lineColor);
        rightLine->SetName("RightLine");
        c->AddControl(rightLine);
        SafeRelease(rightLine);
    }

    UIControl *topLine = c->FindByName("TopLine", false);
    if(!topLine)
    {
        topLine = ControlsFactory::CreateLine(Rect(0, 0, fullRect.dx, 1), lineColor);
        topLine->SetName("TopLine");
        c->AddControl(topLine);
        SafeRelease(topLine);
    }

    UIControl *bottomtLine = c->FindByName("BottomLine", false);
    if(!bottomtLine)
    {
        bottomtLine = ControlsFactory::CreateLine(Rect(0, fullRect.dy-1, fullRect.dx, 1), lineColor);
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

