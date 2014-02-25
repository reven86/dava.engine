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



#include "LandscapeToolsPanel.h"

#include "ControlsFactory.h"
#include "LandscapeTool.h"

#include "EditorSettings.h"


const float32 LandscapeToolsPanel::SLIDER_WIDTH = 250.0f;
const float32 LandscapeToolsPanel::TEXTFIELD_WIDTH = 40.0f;
const float32 LandscapeToolsPanel::TEXT_WIDTH = 60.0f;


LandscapeToolsPanel::LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   UIControl(rect)
    ,   delegate(newDelegate)
    ,   selectedTool(NULL)
    ,   selectedBrushTool(NULL)
{
    ControlsFactory::CustomizeDialogFreeSpace(this);
    
    selectionPanel = NULL;

    brushIcon = new UIControl(Rect(0, 0, ControlsFactory::TOOLS_HEIGHT, ControlsFactory::TOOLS_HEIGHT));
    brushIcon->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanel::OnBrushTool));
    brushIcon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    AddControl(brushIcon);
    
    
    sizeSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - TEXTFIELD_WIDTH,
                                   0, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    sizeSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanel::OnSizeChanged));
    AddControl(sizeSlider);
    
    strengthSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - TEXTFIELD_WIDTH, 
                                       ControlsFactory::TOOLS_HEIGHT / 2, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    strengthSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanel::OnStrengthChanged));
    AddControl(strengthSlider);
    
    AddSliderHeader(sizeSlider, LocalizedString(L"landscapeeditor.size"));
    AddSliderHeader(strengthSlider, LocalizedString(L"landscapeeditor.strength"));
}

LandscapeToolsPanel::LandscapeToolsPanel(const Rect & rect):UIControl(rect)
{
};

LandscapeToolsPanel::~LandscapeToolsPanel()
{
    SafeRelease(strengthSlider);
    SafeRelease(sizeSlider);

    SafeRelease(brushIcon);
}

UISlider * LandscapeToolsPanel::CreateSlider(const Rect & rect)
{
    //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    Texture::SetDefaultGPU(GPU_UNKNOWN);

    UISlider *slider = new UISlider(rect);
    slider->SetMinMaxValue(0.f, 1.0f);
    slider->SetValue(0.5f);
    
    slider->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    slider->SetMinDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMinLeftRightStretchCap(5);

    slider->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    slider->SetMaxDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    slider->SetMaxLeftRightStretchCap(5);

    slider->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    
    Texture::SetDefaultGPU(EditorSettings::Instance()->GetTextureViewGPU());
    
    return slider;
}

void LandscapeToolsPanel::AddSliderHeader(UISlider *slider, const WideString &text)
{
    Rect rect = slider->GetRect();
    rect.x -= rect.dx - OFFSET;
    
    UIStaticText *textControl = new UIStaticText(rect);
    textControl->SetName(WStringToString(text));
    textControl->SetText(text);
    textControl->SetFont(ControlsFactory::GetFont12());
	textControl->SetTextColor(ControlsFactory::GetColorLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_RIGHT);
    AddControl(textControl);
    SafeRelease(textControl);
}

UICheckBox *LandscapeToolsPanel::CreateCkeckbox(const Rect &rect, const WideString &text)
{
    UICheckBox *checkbox = new UICheckBox("~res:/Gfx/UI/chekBox", rect);
    checkbox->SetDelegate(this);
    AddControl(checkbox);
    
    Rect textRect;
    textRect.x = rect.x + rect.dx + OFFSET;
    textRect.y = rect.y;
    textRect.dx = TEXT_WIDTH;
    textRect.dy = rect.dy;
    
    UIStaticText *textControl = new UIStaticText(textRect);
    textControl->SetText(text);
    textControl->SetFont(ControlsFactory::GetFont12());
	textControl->SetTextColor(ControlsFactory::GetColorLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_LEFT);
    AddControl(textControl);
    SafeRelease(textControl);
    
    return checkbox;
}


LandscapeTool * LandscapeToolsPanel::CurrentTool()
{
    return selectedTool;
}

void LandscapeToolsPanel::SetSelectionPanel(LandscapeToolsSelection *newPanel)
{
    selectionPanel = newPanel;
	selectionPanel->SetVisible(this->GetVisible());
    if(selectionPanel)
    {
        selectionPanel->SetDelegate(this);
    }
}


void LandscapeToolsPanel::OnBrushTool(DAVA::BaseObject *, void *, void *)
{
    if(selectedTool == selectedBrushTool)
    {
        if(selectionPanel)
        {
            selectionPanel->Show();
        }
    }
    else 
    {
        selectedTool = selectedBrushTool;
        if(delegate)
        {
            delegate->OnToolSelected(selectedTool);
        }
    }
    
    ToolIconSelected(brushIcon);
}

void LandscapeToolsPanel::WillAppear()
{
    if(selectionPanel)
    {
        selectedBrushTool = selectionPanel->Tool();
        selectedTool = selectedBrushTool;
        if(selectedBrushTool)
        {
            brushIcon->SetSprite(selectedBrushTool->sprite, 0);
        }
        else 
        {
            brushIcon->SetSprite(NULL, 0);
        }
        ToolIconSelected(brushIcon);
    }
    
    UpdateRect();
}

void LandscapeToolsPanel::Input(DAVA::UIEvent *currentInput)
{
    if(UIEvent::PHASE_KEYCHAR == currentInput->phase)
    { 
       if('+' == currentInput->keyChar) 
       {
           float32 sz = sizeSlider->GetValue();
           float32 maxVal = sizeSlider->GetMaxValue();
           
           sz += maxVal * 0.05f;
           sizeSlider->SetValue(Min(sz, maxVal));
           sizeSlider->PerformEvent(UIControl::EVENT_VALUE_CHANGED);
       }
       else if('-' == currentInput->keyChar)
       {
           float32 sz = sizeSlider->GetValue();
           float32 maxVal = sizeSlider->GetMaxValue();
           
           sz -= maxVal * 0.05f;
           sizeSlider->SetValue(Max(sz, sizeSlider->GetMinValue()));
           sizeSlider->PerformEvent(UIControl::EVENT_VALUE_CHANGED);
       }
    }
}

void LandscapeToolsPanel::OnStrengthChanged(DAVA::BaseObject *, void *, void *)
{
    if(selectedBrushTool)
    {
        selectedBrushTool->strength = strengthSlider->GetValue();
    }
}

void LandscapeToolsPanel::OnSizeChanged(DAVA::BaseObject *, void *, void *)
{
    if(selectedBrushTool)
    {
        selectedBrushTool->size = sizeSlider->GetValue();
    }
}

void LandscapeToolsPanel::ToolIconSelected(UIControl *focused)
{
    brushIcon->SetDebugDraw(focused == brushIcon);
}


void LandscapeToolsPanel::OnToolSelected(LandscapeToolsSelection * , LandscapeTool *newTool)
{
    DVASSERT(newTool);
    
    newTool->size = sizeSlider->GetValue();
    newTool->strength = strengthSlider->GetValue();

    
    selectedBrushTool = newTool;
    selectedTool = selectedBrushTool;
    brushIcon->SetSprite(selectedBrushTool->sprite, 0);
    
    if(delegate)
    {
        delegate->OnToolSelected(newTool);
    }
}

void LandscapeToolsPanel::ValueChanged(UICheckBox *, bool )
{
}


void LandscapeToolsPanel::UpdateRect()
{
    UIScreen *activeScreen = UIScreenManager::Instance()->GetScreen();
    if(activeScreen)
    {
        Vector2 screenSize = activeScreen->GetSize();
        
        Vector2 panelSize = this->GetSize();
        Vector2 panelPosition = this->GetPosition();

        this->SetSize(Vector2(screenSize.x - EditorSettings::Instance()->GetRightPanelWidth(), panelSize.y));
        this->SetPosition(Vector2(0, panelPosition.y));
    }
}

void LandscapeToolsPanel::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);
    
    sizeSlider->SetPosition(Vector2(newSize.x - SLIDER_WIDTH - TEXTFIELD_WIDTH, 0));
    strengthSlider->SetPosition(Vector2(newSize.x - SLIDER_WIDTH - TEXTFIELD_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));

    SetSliderHeaderPoition(sizeSlider, LocalizedString(L"landscapeeditor.size"));
    SetSliderHeaderPoition(strengthSlider, LocalizedString(L"landscapeeditor.strength"));
}

void LandscapeToolsPanel::SetSliderHeaderPoition(UISlider *slider, const WideString &headerText)
{
    UIStaticText *textControl = static_cast<UIStaticText *>(this->FindByName(WStringToString(headerText)));
    if(textControl)
    {
        Rect sliderRect = slider->GetRect();
        Vector2 textPosition = textControl->GetPosition();

        textControl->SetPosition(Vector2(sliderRect.x - OFFSET - sliderRect.dx, textPosition.y));
    }
}

