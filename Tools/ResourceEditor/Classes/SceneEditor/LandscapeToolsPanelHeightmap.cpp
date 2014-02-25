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



#include "LandscapeToolsPanelHeightmap.h"
#include "ControlsFactory.h"
#include "LandscapeTool.h"
#include "../Qt/Main/QtUtils.h"

LandscapeToolsPanelHeightmap::LandscapeToolsPanelHeightmap(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   LandscapeToolsPanel(newDelegate, rect)
{
    Rect sizeRect(rect.dx - TEXTFIELD_WIDTH, 
                  0, TEXTFIELD_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2);
    sizeValue = CreateTextField(sizeRect);
    AddControl(sizeValue);
    
    Rect strengthRect(sizeRect);
    strengthRect.y = sizeRect.y + sizeRect.dy;
    strengthValue = CreateTextField(strengthRect);
    AddControl(strengthValue);
    
    Rect lineRect;
    lineRect.x = strengthSlider->GetRect().x + strengthSlider->GetRect().dx/2;
    lineRect.dx = 2;
    lineRect.y = strengthSlider->GetRect().y - 2;
    lineRect.dy = strengthSlider->GetRect().dy + 4;
    line = ControlsFactory::CreateLine(lineRect);
    AddControl(line);

    showGrid = CreateCkeckbox(Rect(0, ControlsFactory::TOOLS_HEIGHT, ControlsFactory::TOOLS_HEIGHT/2, ControlsFactory::TOOLS_HEIGHT/2), 
                              LocalizedString(L"landscapeeditor.showgrid"));
    
    float32 x = 0;
    float32 y = ControlsFactory::TOOLS_HEIGHT + ControlsFactory::TOOLS_HEIGHT/2.f;
    relative = CreateCkeckbox(Rect(x, y, ControlsFactory::TOOLS_HEIGHT/2.f, ControlsFactory::TOOLS_HEIGHT/2.f), 
                              LocalizedString(L"landscapeeditor.relative"));
    x += (ControlsFactory::TOOLS_HEIGHT/2.f + OFFSET + TEXT_WIDTH);
    average = CreateCkeckbox(Rect(x, y, ControlsFactory::TOOLS_HEIGHT/2.f, ControlsFactory::TOOLS_HEIGHT/2.f), 
                              LocalizedString(L"landscapeeditor.average"));
    
    x += (ControlsFactory::TOOLS_HEIGHT/2.f + OFFSET + TEXT_WIDTH);
    absoluteDropper = CreateCkeckbox(Rect(x, y, ControlsFactory::TOOLS_HEIGHT/2.f, ControlsFactory::TOOLS_HEIGHT/2.f), 
                             LocalizedString(L"landscapeeditor.absolutedropper"));

    Rect heightRect;
    heightRect.x = showGrid->GetRect().x + showGrid->GetRect().dx + TEXT_WIDTH + OFFSET;
    heightRect.y = ControlsFactory::TOOLS_HEIGHT;
    heightRect.dx = TEXTFIELD_WIDTH;
    heightRect.dy = average->GetRect().dy;
    heightValue = CreateTextField(Rect(heightRect));
    heightValue->SetText(L"0");
    AddControl(heightValue);
    
    heightRect.x = heightRect.x + heightRect.dx + ControlsFactory::OFFSET/2.f;
    heightRect.dx = TEXT_WIDTH;
    
    UIStaticText *textControl = new UIStaticText(heightRect);
    textControl->SetText(LocalizedString(L"landscapeeditor.height"));
    textControl->SetFont(ControlsFactory::GetFont12());
	textControl->SetTextColor(ControlsFactory::GetColorLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_LEFT);
    AddControl(textControl);
    SafeRelease(textControl);
    
    averageStrength = CreateSlider(Rect(rect.dx - SLIDER_WIDTH, heightRect.y, 
                                        SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    averageStrength->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanelHeightmap::OnAverageSizeChanged));
    AddControl(averageStrength);
    AddSliderHeader(averageStrength, LocalizedString(L"landscapeeditor.averagestrength"));

    
    dropperTool = new LandscapeTool(-1, LandscapeTool::TOOL_DROPPER, "~res:/LandscapeEditor/SpecialTools/dropper.png");
    dropperTool->size = 1.0f;
    dropperTool->height = prevHeightValue = 0.f;
    
    Rect dropperRect = brushIcon->GetRect();
    dropperRect.x = (dropperRect.x + dropperRect.dx + ControlsFactory::OFFSET);
    dropperIcon = new UIControl(dropperRect);
    dropperIcon->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanelHeightmap::OnDropperTool));
    dropperIcon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    dropperIcon->SetSprite(dropperTool->sprite, 0);
    AddControl(dropperIcon);
    

    copypasteTool = new LandscapeTool(-1, LandscapeTool::TOOL_COPYPASTE, "~res:/LandscapeEditor/SpecialTools/copypaste.png");
    
    Rect copypasteRect = dropperRect;
    copypasteRect.x = (copypasteRect.x + copypasteRect.dx + ControlsFactory::OFFSET);
    copypasteIcon = new UIControl(copypasteRect);
    copypasteIcon->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LandscapeToolsPanelHeightmap::OnCopypasteTool));
    copypasteIcon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    copypasteIcon->SetSprite(copypasteTool->sprite, 0);
    AddControl(copypasteIcon);
    
    copypasteRect.x = (copypasteRect.x + copypasteRect.dx + ControlsFactory::OFFSET);
    copypasteRect.dx = copypasteRect.dy = ControlsFactory::TOOLS_HEIGHT / 2;
    copyHeightmap = CreateCkeckbox(copypasteRect, L"Height");
    copypasteRect.y += copypasteRect.dy;
    copyTilemask = CreateCkeckbox(copypasteRect, L"Tilemask");
    
    sizeValue->SetText(Format(L"%.3f", LandscapeTool::SizeHeightMax()));
    strengthValue->SetText(Format(L"%.3f", LandscapeTool::StrengthHeightMax()));
    
    relative->SetChecked(true, false);
    average->SetChecked(false, false);
    absoluteDropper->SetChecked(false, false);

    sizeSlider->SetMinMaxValue(0.f, LandscapeTool::SizeHeightMax());
    sizeSlider->SetValue(LandscapeTool::DefaultSizeHeight());

    strengthSlider->SetMinMaxValue(-LandscapeTool::StrengthHeightMax(), LandscapeTool::StrengthHeightMax());
    strengthSlider->SetValue(LandscapeTool::DefaultStrengthHeight());
    
    averageStrength->SetMinMaxValue(0.f, 1.f);
    averageStrength->SetValue(0.5f);
    
    copyHeightmap->SetChecked(true, false);    
    copyTilemask->SetChecked(true, false);    
}


LandscapeToolsPanelHeightmap::~LandscapeToolsPanelHeightmap()
{
    SafeRelease(line);

    
    SafeRelease(copypasteIcon);
    SafeRelease(copypasteTool);
    
    SafeRelease(dropperIcon);
    SafeRelease(dropperTool);

    SafeRelease(heightValue);
    
    SafeRelease(average);
    SafeRelease(relative);
    
    SafeRelease(sizeValue);
    SafeRelease(strengthValue);
}

void LandscapeToolsPanelHeightmap::WillAppear()
{
    LandscapeToolsPanel::WillAppear();
        
    showGrid->SetChecked(showGrid->Checked(), true);
    
    copyHeightmap->SetChecked(copyHeightmap->Checked(), true);    
    copyTilemask->SetChecked(copyTilemask->Checked(), true);    
}

void LandscapeToolsPanelHeightmap::Input(DAVA::UIEvent *currentInput)
{
    if(UIEvent::PHASE_KEYCHAR == currentInput->phase)
    {
        if(IsKeyModificatorPressed(DVKEY_CTRL))
        {
            if(DVKEY_1 == currentInput->tid)
            {
                relative->SetChecked(true, true);
            }
            if(DVKEY_2 == currentInput->tid)
            {
                average->SetChecked(true, true);
            }
            if(DVKEY_3 == currentInput->tid)
            {
                absoluteDropper->SetChecked(true, true);
            }
        }
    }
    
    LandscapeToolsPanel::Input(currentInput);
}


UITextField *LandscapeToolsPanelHeightmap::CreateTextField(const Rect &rect)
{
    Font * font = ControlsFactory::GetFont12();
    UITextField *tf = new UITextField(rect);
    ControlsFactory::CustomizeEditablePropertyCell(tf);
    tf->SetFont(font);
	tf->SetTextColor(ControlsFactory::GetColorLight());
    tf->SetDelegate(this);
    
    return tf;
}

void LandscapeToolsPanelHeightmap::OnDropperTool(DAVA::BaseObject *, void *, void *)
{
    selectedTool = dropperTool;
    if(delegate)
    {
        delegate->OnToolSelected(selectedTool);
    }
    
    if(average->Checked())
    {
        average->SetChecked(false, true);
    }
    if(relative->Checked())
    {
        relative->SetChecked(false, true);
    }
    if(absoluteDropper->Checked())
    {
        absoluteDropper->SetChecked(false, true);
    }
    
    ToolIconSelected(dropperIcon);
}

void LandscapeToolsPanelHeightmap::OnCopypasteTool(DAVA::BaseObject *, void *, void *)
{
    copypasteTool->size = sizeSlider->GetValue();
    copypasteTool->strength = strengthSlider->GetValue();
    copypasteTool->averageStrength = averageStrength->GetValue();
    
    SafeRelease(copypasteTool->sprite);
    SafeRelease(copypasteTool->image);
    if(selectedBrushTool)
    {
        copypasteTool->sprite = SafeRetain(selectedBrushTool->sprite);
        copypasteTool->image = SafeRetain(selectedBrushTool->image);
    }
    
    
    selectedTool = copypasteTool;
    
    if(delegate)
    {
        delegate->OnToolSelected(selectedTool);
    }
    
    if(average->Checked())
    {
        average->SetChecked(false, true);
    }
    if(relative->Checked())
    {
        relative->SetChecked(false, true);
    }
    if(absoluteDropper->Checked())
    {
        absoluteDropper->SetChecked(false, true);
    }
    
    ToolIconSelected(copypasteIcon);
}


void LandscapeToolsPanelHeightmap::OnAverageSizeChanged(DAVA::BaseObject *, void *, void *)
{
    selectedTool->averageStrength = averageStrength->GetValue();
}

void LandscapeToolsPanelHeightmap::Update(float32 timeElapsed)
{
    if(selectedTool->height != prevHeightValue)
    {
        prevHeightValue = selectedTool->height;
        heightValue->SetText(Format(L"%.3f", selectedTool->height));
        
        if(selectedBrushTool)
        {
            selectedBrushTool->height = selectedTool->height;
        }
        dropperTool->height = selectedTool->height;
    }
    
    LandscapeToolsPanel::Update(timeElapsed);
}

void LandscapeToolsPanelHeightmap::ToolIconSelected(UIControl *focused)
{
    dropperIcon->SetDebugDraw(focused == dropperIcon);
    copypasteIcon->SetDebugDraw(focused == copypasteIcon);
    LandscapeToolsPanel::ToolIconSelected(focused);
}

void LandscapeToolsPanelHeightmap::OnSizeChanged(BaseObject * object, void * userData, void * callerData)
{
    if(copypasteTool == selectedTool)
    {
        copypasteTool->size = sizeSlider->GetValue();
    }
    
    LandscapeToolsPanel::OnSizeChanged(object, userData, callerData);
}

void LandscapeToolsPanelHeightmap::OnStrengthChanged(BaseObject * object, void * userData, void * callerData)
{
    if(copypasteTool == selectedTool)
    {
        copypasteTool->strength = strengthSlider->GetValue();
    }
    
    LandscapeToolsPanel::OnStrengthChanged(object, userData, callerData);
}



void LandscapeToolsPanelHeightmap::TextFieldShouldReturn(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LandscapeToolsPanelHeightmap::TextFieldShouldCancel(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LandscapeToolsPanelHeightmap::TextFieldLostFocus(UITextField * textField)
{
    if(textField == sizeValue)
    {
        float32 value = (float32)atof(WStringToString(sizeValue->GetText()).c_str());
        
        sizeSlider->SetMinMaxValue(0.f, value);
        selectedBrushTool->maxSize = value;
        if(value < selectedBrushTool->size)
        {
            selectedBrushTool->size = value;
        }
        sizeSlider->SetValue(selectedBrushTool->size);
    }
    else if(textField == strengthValue)
    {
        float32 value = (float32)fabsf((float32)atof(WStringToString(strengthValue->GetText()).c_str()));

        strengthSlider->SetMinMaxValue(-value, value);
        selectedBrushTool->maxStrength = value;
        if(value < selectedBrushTool->strength)
        {
            selectedBrushTool->strength = value;
        }
        else if(selectedBrushTool->strength < -value)
        {
            selectedBrushTool->strength = -value;
        }
        strengthSlider->SetValue(selectedBrushTool->strength);
    }
    else if(textField == heightValue)
    {
        prevHeightValue = (float32)fabsf((float32)atof(WStringToString(heightValue->GetText()).c_str()));
        dropperTool->height = prevHeightValue;

        if(selectedBrushTool)
        {
            selectedBrushTool->height = prevHeightValue;
        }
    }
}

bool LandscapeToolsPanelHeightmap::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (replacementLength < 0) 
    {
        return true;
    }

    WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
    bool allOk;
    int pointsCount = 0;
    for (int i = 0; i < (int32)newText.length(); i++)
    {
        allOk = false;
        if (newText[i] == L'-' && i == 0)
        {
            allOk = true;
        }
        else if(newText[i] >= L'0' && newText[i] <= L'9')
        {
            allOk = true;
        }
        else if(newText[i] == L'.' && pointsCount == 0)
        {
            allOk = true;
            pointsCount++;
        }
        if (!allOk) 
        {
            return false;
        }
    }
    return true;
};

void LandscapeToolsPanelHeightmap::ValueChanged(UICheckBox *forCheckbox, bool newValue)
{
    LandscapeToolsPanel::ValueChanged(forCheckbox, newValue);

    if(forCheckbox == average)
    {
        if(newValue)
        {
            relative->SetChecked(false, false);
            absoluteDropper->SetChecked(false, false);
        }
    }
    else if(forCheckbox == relative)
    {
        if(newValue)
        {
            average->SetChecked(false, false);
            absoluteDropper->SetChecked(false, false);
        }
    }
    else if(forCheckbox == absoluteDropper)
    {
        if(newValue)
        {
            relative->SetChecked(false, false);
            average->SetChecked(false, false);
        }
    }
    else if(forCheckbox == showGrid)
    {
        if(delegate)
        {
            delegate->OnShowGrid(showGrid->Checked());
        }
    }
    else if(forCheckbox == copyTilemask)
    {
        copypasteTool->copyTilemask = newValue;
    }
    else if(forCheckbox == copyHeightmap)
    {
        copypasteTool->copyHeightmap = newValue;
    }

    if(selectedBrushTool)
    {
        selectedBrushTool->averageDrawing = average->Checked();
        selectedBrushTool->relativeDrawing = relative->Checked();
        selectedBrushTool->absoluteDropperDrawing = absoluteDropper->Checked();
    }
}

void LandscapeToolsPanelHeightmap::OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool)
{
    newTool->height = (float32)atof(WStringToString(heightValue->GetText()).c_str());
    newTool->averageDrawing = average->Checked();
    newTool->relativeDrawing = relative->Checked();
    
    newTool->maxStrength = (float32)fabsf((float32)atof(WStringToString(strengthValue->GetText()).c_str()));
    newTool->maxSize = (float32)fabsf((float32)atof(WStringToString(sizeValue->GetText()).c_str()));
    newTool->averageStrength = averageStrength->GetValue();

    LandscapeToolsPanel::OnToolSelected(forControl, newTool);
}

void LandscapeToolsPanelHeightmap::SetSize(const Vector2 &newSize)
{
    LandscapeToolsPanel::SetSize(newSize);
    
    line->SetPosition(Vector2(strengthSlider->GetRect().x + strengthSlider->GetRect().dx/2, strengthSlider->GetRect().y - 2));

    averageStrength->SetPosition(Vector2(newSize.x - SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT));
    SetSliderHeaderPoition(averageStrength, LocalizedString(L"landscapeeditor.averagestrength"));

    sizeValue->SetPosition(Vector2(newSize.x - sizeValue->GetSize().dx, 0));
    strengthValue->SetPosition(Vector2(newSize.x - strengthValue->GetSize().dx, strengthValue->GetPosition().y));
}
