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


#ifndef __ALIGN__HANDLERS__H__
#define __ALIGN__HANDLERS__H__

#include "DAVAEngine.h"
#include "ControlsPositionData.h"

namespace DAVA {
	
// Align Handlers for UI Editor.
// Base handler
class BaseAlignHandler
{
public:
	// Do the alignment.
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList) = 0;
	
protected:
	// Whether to use first or last control for centering?
	// Implemented as method just for quick adjusting.
	bool IsUseLastControlForCentering() {return true;};
	
	// Determine the reference control (first or last depending on previous flag.
	UIControl* GetReferenceControl(const List<UIControl*>& controlsList);

	// Align by left or top.
	ControlsPositionData AlignLeftTop(const List<UIControl*>& controlsList, bool isLeft);

	// Align by center (horizontal or vertical).
	ControlsPositionData AlignCenter(const List<UIControl*>& controlsList, bool isHorizontal);

	// Align by right or bottom.
	ControlsPositionData AlignRightBottom(const List<UIControl*>& controlsList, bool isRight);
};

// Simpliest "do nothing" align handler, which doesn't change the positions of controls.
class DoNothingAlignHandler : public BaseAlignHandler
{
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList);
};

// "Align on the left" handler.
class AlignLeftHandler : public BaseAlignHandler
{
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList);
};

// "Align on the horizontal center" handler.
class AlignHorzCenterHandler : public BaseAlignHandler
{
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList);
};

// "Align on the right" handler.
class AlignRightHandler : public BaseAlignHandler
{
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList);
};

// "Align on the top" handler.
class AlignTopHandler : public BaseAlignHandler
{
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList);
};
	
// "Align on the vertical center" handler.
class AlignVertCenterHandler : public BaseAlignHandler
{
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList);
};
	
// "Align on the bottom" handler.
class AlignBottomHandler : public BaseAlignHandler
{
	virtual ControlsPositionData Align(const List<UIControl*>& controlsList);
};

};

#endif /* defined(__ALIGN__HANDLERS__H__) */
