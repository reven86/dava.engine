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


#ifndef __DISTRIBUTE__HANDLERS__H__
#define __DISTRIBUTE__HANDLERS__H__

#include "DAVAEngine.h"
#include "ControlsPositionData.h"

namespace DAVA {

// Distribute Handlers for UI Editor.
// Base handler
class BaseDistributeHandler
{
public:
	// Do the distribute.
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList) = 0;
	
protected:
	// Build the position data.
	ControlsPositionData BuildPositionData(const List<UIControl*>& controlsList);

	// Sorting helpers.
	static bool SortByXPosition(UIControl* control1, UIControl* control2);
	static bool SortByYPosition(UIControl* control1, UIControl* control2);
	
	// Distribution helpers.
	ControlsPositionData OrderControlsList(List<UIControl*>& orderedControlsList, bool isHorizontal, bool& canProcess);
	
	ControlsPositionData DistributeLeftTopEdges(const List<UIControl*>& controlsList, bool isHorizontal);
	ControlsPositionData DistributeXYCenter(const List<UIControl*>& controlsList, bool isHorizontal);
	ControlsPositionData DistributeRightBottomEdges(const List<UIControl*>& controlsList, bool isHorizontal);
	ControlsPositionData DistributeXY(const List<UIControl*>& controlsList, bool isHorizontal);
};

// Simpliest "do nothing" distribute handler, which doesn't change the positions of controls.
class DoNothingDistributeHandler : public BaseDistributeHandler
{
public:
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};

// Align the controls in the way to have equal distances between their left edges.
class DistributeEqualDistanceLeftEdgesHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};
	
// Align the controls in the way to have equal distances between X centers.
class DistributeEqualDistanceXCentersHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};

// Align the controls in the way to have equal distances between their right edges.
class DistributeEqualDistanceRightEdgesHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};

// Align the controls in the way to have equal X distances between them.
class DistributeEqualDistanceXHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};

// Align the controls in the way to have equal distances between their top edges.
class DistributeEqualDistanceTopEdgesHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};
	
// Align the controls in the way to have equal distances between Y centers.
class DistributeEqualDistanceYCentersHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};

// Align the controls in the way to have equal distances between their bottom edges.
class DistributeEqualDistanceBottomEdgesHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};

// Align the controls in the way to have equal Y distances between them.
class DistributeEqualDistanceYHandler : public BaseDistributeHandler
{
	virtual ControlsPositionData Distribute(const List<UIControl*>& controlsList);
};

};

#endif /* defined(__DISTRIBUTE__HANDLERS__H__) */
