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



#ifndef __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__
#define __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeTool.h"

using namespace DAVA;

class LandscapeToolsPanelHeightmap: 
    public LandscapeToolsPanel, 
    public UITextFieldDelegate
{

public:
    LandscapeToolsPanelHeightmap(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanelHeightmap();
    
    virtual void WillAppear();
    virtual void Update(float32 timeElapsed);

    virtual void Input(UIEvent *currentInput);

    
    //UITextFieldDelegate
    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, 
                                     int32 replacementLength, const WideString & replacementString);


    //UICheckBoxDelegate
    virtual void ValueChanged(UICheckBox *forCheckbox, bool newValue);

    //LandscapeToolsSelectionDelegate
    virtual void OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool);

    virtual void SetSize(const Vector2 &newSize);

protected:

    void OnDropperTool(BaseObject * object, void * userData, void * callerData);
    void OnCopypasteTool(BaseObject * object, void * userData, void * callerData);
    virtual void ToolIconSelected(UIControl *focused);

    virtual void OnSizeChanged(BaseObject * object, void * userData, void * callerData);
	virtual void OnStrengthChanged(BaseObject * object, void * userData, void * callerData);

    
    UITextField *sizeValue;
    UITextField *strengthValue;
    
    UITextField *CreateTextField(const Rect &rect);
    
    UICheckBox *relative;
    UICheckBox *average;
    UICheckBox *absoluteDropper;
    
    UITextField *heightValue;
    
    UIControl *dropperIcon;
    LandscapeTool *dropperTool;

    UIControl *copypasteIcon;
    LandscapeTool *copypasteTool;
    UICheckBox *copyHeightmap;
    UICheckBox *copyTilemask;

    
    UISlider *averageStrength;
    void OnAverageSizeChanged(BaseObject * object, void * userData, void * callerData);

    UICheckBox *showGrid;
    
    float32 prevHeightValue;
    
    UIControl *line;
};

#endif // __LANDSCAPE_TOOLS_PANEL_HEIGHTMAP_H__