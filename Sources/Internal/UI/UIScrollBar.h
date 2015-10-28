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


#ifndef __DAVAENGINE_UI_SCROLL_BAR_H__
#define __DAVAENGINE_UI_SCROLL_BAR_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

#define MINIMUM_SLIDER_SIZE     30

namespace DAVA 
{

class UIScrollBar;
class UIScrollBarDelegate 
{
public:
    friend class UIScrollBar;
    virtual float32 VisibleAreaSize(UIScrollBar *forScrollBar) = 0;
    virtual float32 TotalAreaSize(UIScrollBar *forScrollBar) = 0;
    virtual float32 ViewPosition(UIScrollBar *forScrollBar) = 0;
    virtual void OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition) = 0;
    virtual const String GetDelegateControlPath(const UIControl *rootControl) const {return String(); }; // TODO!! TEMP SOLUTION, CHECK WITH AUTHOR!
};



class UIScrollBar : public UIControl
{//TODO: add top and bottom buttons
public:
    enum eScrollOrientation 
    {
            ORIENTATION_VERTICAL = 0
        ,	ORIENTATION_HORIZONTAL
    };
protected:
    virtual ~UIScrollBar();
public:
    UIScrollBar(const Rect& rect = Rect(), eScrollOrientation requiredOrientation = ORIENTATION_VERTICAL);

    void SetDelegate(UIScrollBarDelegate *newDelegate);
    const String GetDelegatePath(const UIControl *rootControl) const;
    UIControl *GetSlider();
    
    virtual void Draw(const UIGeometricData &geometricData);
	virtual void AddControl(UIControl *control);
    virtual void RemoveControl(UIControl *control);
    UIScrollBar* Clone() override;
    virtual void CopyDataFrom(UIControl* srcControl);

    virtual void LoadFromYamlNode(const YamlNode* node, UIYamlLoader* loader);
    virtual void LoadFromYamlNodeCompleted();
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);
	
    void Input(UIEvent *currentInput);

	int32 GetOrientation() const;
	void SetOrientation(int32 value);

protected:
	// Calculate the start offset based on the initial click point.
	void CalculateStartOffset(const Vector2& inputPoint);
	void InitControls(const Rect &rect = Rect());

private:
    eScrollOrientation orientation;
    UIScrollBarDelegate *delegate;
    
    UIControl *slider;
    
    bool resizeSliderProportionally;

	Vector2 startPoint;
	Vector2 startOffset;
	
	float32 GetValidSliderSize(float32 size);
public:
    INTROSPECTION_EXTEND(UIScrollBar, UIControl,
        PROPERTY("orientation",  InspDesc("Bar orientation", GlobalEnumMap<UIScrollBar::eScrollOrientation>::Instance()), GetOrientation, SetOrientation, I_SAVE | I_VIEW | I_EDIT)
    );
};



};



#endif