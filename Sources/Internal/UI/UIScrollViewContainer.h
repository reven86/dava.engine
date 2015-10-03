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


#ifndef __DAVAENGINE_UI_SCROLLVIEWCONTAINER__
#define __DAVAENGINE_UI_SCROLLVIEWCONTAINER__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA 
{
class ScrollHelper;

class UIScrollViewContainer : public UIControl
{
protected:
	virtual ~UIScrollViewContainer();
public:
    UIScrollViewContainer(const Rect &rect = Rect());

    virtual UIScrollViewContainer *Clone();
	virtual void CopyDataFrom(UIControl *srcControl);
	
public:
	virtual void Update(float32 timeElapsed);
	virtual void Input(UIEvent *currentTouch);
    virtual void InputCancelled(UIEvent *currentInput);
	virtual bool SystemInput(UIEvent *currentInput);
	virtual void SetRect(const Rect &rect);
    virtual void WillDisappear();


	// The amount of pixels user must move the finger on the button to switch from button to scrolling (default 15)
	void SetTouchTreshold(int32 holdDelta);
	int32 GetTouchTreshold();


protected:

	enum
	{
		STATE_NONE = 0,
		STATE_SCROLL,
		STATE_ZOOM,
		STATE_DECCELERATION,
		STATE_SCROLL_TO_SPECIAL,
	};

	int32		state;
	// Scroll information
	Vector2		scrollStartInitialPosition;	// position of click
	int32		touchTreshold;
	
	int 		mainTouch;	
	
	Vector2 	oldPos;
	Vector2		newPos;

    ScrollHelper *currentScroll;

	// All boolean variables are grouped together because of DF-2149.
	bool 		lockTouch : 1;
	bool 		scrollStartMovement : 1;
	bool		enableHorizontalScroll : 1;
	bool		enableVerticalScroll : 1;
};
};

#endif /* defined(__DAVAENGINE_UI_SCROLLVIEWCONTAINER__) */
