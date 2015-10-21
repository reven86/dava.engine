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


#ifndef __DAVAENGINE_UISCROLLVIEW_H__
#define __DAVAENGINE_UISCROLLVIEW_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIScrollBar.h"

namespace DAVA
{

class UIScrollViewContainer;
class ScrollHelper;

class UIScrollView : public UIControl, public UIScrollBarDelegate
{
public:
    UIScrollView(const Rect &rect = Rect());

protected:
    virtual ~UIScrollView();

public:
	virtual void AddControl(UIControl *control);
    virtual void RemoveControl(UIControl *control);
    
	// Add the control directly to the Scroll View Container.
	void AddControlToContainer(UIControl* control);

	// Access to the Scroll View Container.
	UIScrollViewContainer* GetContainer();
	ScrollHelper* GetHorizontalScroll();
	ScrollHelper* GetVerticalScroll();
	
    // Scroll Position getter/setters.
    float32 GetHorizontalScrollPosition() const;
    float32 GetVerticalScrollPosition() const;
    Vector2 GetScrollPosition() const;

    void SetHorizontalScrollPosition(float32 horzPos);
    void SetVerticalScrollPosition(float32 vertPos);
    void SetScrollPosition(const Vector2& pos);

    void ScrollToHorizontalPosition(float32 horzPos, float32 timeSec = 0.3f);
    void ScrollToVerticalPosition(float32 vertPos, float32 timeSec = 0.3f);
    void ScrollToPosition(const Vector2& pos, float32 timeSec = 0.3f);

    UIScrollView *Clone() override;
	virtual void CopyDataFrom(UIControl *srcControl);
	
	virtual void SetRect(const Rect &rect);
	virtual void SetSize(const Vector2 &newSize);
	
	void SetPadding(const Vector2 & padding);
	const Vector2 GetPadding() const;
	
	const Vector2 GetContentSize() const;
	
	void RecalculateContentSize();
	
	//Sets how fast scroll container will return to its bounds
	void SetReturnSpeed(float32 speedInSeconds);
	//Sets how fast scroll speed will be reduced
	void SetScrollSpeed(float32 speedInSeconds);

	// UIScrollBarDelegate implementation.
	virtual float32 VisibleAreaSize(UIScrollBar *forScrollBar);
    virtual float32 TotalAreaSize(UIScrollBar *forScrollBar);
    virtual float32 ViewPosition(UIScrollBar *forScrollBar);
    virtual void OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition);
    void OnScrollViewContainerSizeChanged();

    virtual const String GetDelegateControlPath(const UIControl *rootControl) const;
    
    bool IsAutoUpdate() const;
    void SetAutoUpdate(bool auto_);
    
    bool IsCenterContent() const;
    void SetCenterContent(bool center_);

protected:
	virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
    virtual void LoadFromYamlNodeCompleted();
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

	Vector2 GetMaxSize(UIControl *control, Vector2 currentMaxSize, Vector2 parentShift);	
	void PushContentToBounds(UIControl *control);
	Vector2 GetControlOffset(UIControl *control, Vector2 currentContentOffset);

	// Get the X or Y parameter from the vector depending on the scrollbar orientation.
	float32 GetParameterForScrollBar(UIScrollBar* forScrollBar, const Vector2& vectorParam);

	UIScrollViewContainer *scrollContainer;
	ScrollHelper *scrollHorizontal;
	ScrollHelper *scrollVertical;
    
    bool autoUpdate;
    bool centerContent;

private:
	void FindRequiredControls();

public:
    INTROSPECTION_EXTEND(UIScrollView, UIControl,
                         PROPERTY("autoUpdate", "Auto Update", IsAutoUpdate, SetAutoUpdate, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("centerContent", "Center Content", IsCenterContent, SetCenterContent, I_SAVE | I_VIEW | I_EDIT)
                         );

};
};

#endif //__DAVAENGINE_UISCROLLVIEW__