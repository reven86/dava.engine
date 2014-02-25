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



#ifndef __LANDSCAPE_TOOLS_PANEL_H__
#define __LANDSCAPE_TOOLS_PANEL_H__

#include "DAVAEngine.h"
#include "LandscapeToolsSelection.h"
#include "UICheckBox.h"

using namespace DAVA;

class LandscapeToolsPanelDelegate
{
public: 
    
    virtual void OnToolSelected(LandscapeTool *newTool) = 0;
    virtual void OnShowGrid(bool show) = 0;
};

class LandscapeToolsPanel: 
    public UIControl,
    public LandscapeToolsSelectionDelegate,
    public UICheckBoxDelegate
{
protected:
    
    static const int32 OFFSET = 1;
    static const float32 SLIDER_WIDTH;
    static const float32 TEXTFIELD_WIDTH;
    static const float32 TEXT_WIDTH;
    virtual ~LandscapeToolsPanel();
public:
    LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
	LandscapeToolsPanel(const Rect & rect); // for custom panels
    
    virtual void WillAppear();
    virtual void Input(UIEvent *currentInput);

    LandscapeTool *CurrentTool();
    virtual void SetSelectionPanel(LandscapeToolsSelection *newPanel);
    
    //LandscapeToolsSelectionDelegate
    virtual void OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool);

    //UICheckBoxDelegate
    virtual void ValueChanged(UICheckBox *forCheckbox, bool newValue);

    virtual void SetSize(const Vector2 &newSize);
    
protected:

    void UpdateRect();
    void SetSliderHeaderPoition(UISlider *slider, const WideString &headerText);
    
    virtual void ToolIconSelected(UIControl *focused);

    
    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);

    UICheckBox *CreateCkeckbox(const Rect &rect, const WideString &text);

    void OnBrushTool(BaseObject * object, void * userData, void * callerData);

    
    LandscapeToolsPanelDelegate *delegate;
    
    UIControl *brushIcon;
    LandscapeTool *selectedTool;
    LandscapeTool *selectedBrushTool;

    UISlider *sizeSlider;
    UISlider *strengthSlider;
	virtual void OnSizeChanged(BaseObject * object, void * userData, void * callerData);
	virtual void OnStrengthChanged(BaseObject * object, void * userData, void * callerData);
    
    LandscapeToolsSelection *selectionPanel;
    
};

#endif // __LANDSCAPE_TOOLS_PANEL_H__
