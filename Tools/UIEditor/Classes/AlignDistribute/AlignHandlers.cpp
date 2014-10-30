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

#include "AlignHandlers.h"

namespace DAVA {

UIControl* BaseAlignHandler::GetReferenceControl(const List<UIControl*>& controlsList)
{
	UIControl* firstControl = NULL;
	UIControl* lastControl = NULL;
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}
		
		if (!firstControl)
		{
			firstControl = uiControl;
		}

		lastControl = uiControl;
	}

	// Perform the alignment on the first or last control, depending on the flag.
	return IsUseLastControlForCentering() ? lastControl : firstControl;
}

///////////////////////////////////////////////////////////////////////////////////////////

ControlsPositionData BaseAlignHandler::AlignLeftTop(const List<UIControl*>& controlsList, bool isLeft)
{
	ControlsPositionData resultData;

	// Find the reference position. All the alignment is to be done in absolute coords.
	float32 referencePos = FLT_MAX;
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		resultData.AddControl(uiControl);
		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		float32 currentPos = isLeft ? absoluteRect.x : absoluteRect.y;

		if (currentPos < referencePos)
		{
			referencePos = currentPos;
		}
	}

	// Second pass - update.
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		Vector2 moveDelta;
		if(isLeft)
		{
			float32 offsetX = absoluteRect.x - referencePos;
			moveDelta = Vector2(-offsetX, 0);
		}
		else
		{
			float32 offsetY = absoluteRect.y - referencePos;
			moveDelta = Vector2(0, -offsetY);
		}
		Vector2 controlPosition = uiControl->GetPosition(true);
		controlPosition += moveDelta;
		uiControl->SetPosition(controlPosition, true);
	}

	return resultData;
}

///////////////////////////////////////////////////////////////////////////////////////////
	
ControlsPositionData BaseAlignHandler::AlignCenter(const List<UIControl*>& controlsList, bool isHorizontal)
{
	ControlsPositionData resultData;
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		resultData.AddControl(uiControl);
	}

	// Perform the alignment on the first or last control, depending on the flag.
	UIControl* referenceControl = GetReferenceControl(controlsList);
	Vector2 referenceCenter = referenceControl->GetGeometricData().GetAABBox().GetCenter();

	// Second pass - update.
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		Vector2 currentCenter = absoluteRect.GetCenter();
		Vector2 moveDelta;
		if (isHorizontal)
		{
			float32 offsetX = currentCenter.x - referenceCenter.x;
			moveDelta = Vector2(-offsetX, 0);
		}
		else
		{
			float32 offsetY = currentCenter.y - referenceCenter.y;
			moveDelta = Vector2(0, -offsetY);
		}
		Vector2 controlPosition = uiControl->GetPosition(true);
		controlPosition += moveDelta;
		uiControl->SetPosition(controlPosition, true);
	}

	return resultData;
}

ControlsPositionData BaseAlignHandler::AlignRightBottom(const List<UIControl*>& controlsList, bool isRight)
{
	ControlsPositionData resultData;

	// Find the bottom/right position. All the alignment is to be done in absolute coords.
	float32 referencePos = FLT_MIN;
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		resultData.AddControl(uiControl);
		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		float32 controlSize = isRight ? (absoluteRect.x + absoluteRect.dx) : (absoluteRect.y + absoluteRect.dy);
		if (controlSize > referencePos)
		{
			referencePos = controlSize;
		}
	}

	// Second pass - update.
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		Vector2 moveDelta;
		if (isRight)
		{
			float32 offsetX = referencePos - (absoluteRect.x + absoluteRect.dx);
			moveDelta = Vector2(offsetX, 0);
		}
		else
		{
			float32 offsetY = referencePos - (absoluteRect.y + absoluteRect.dy);
			moveDelta = Vector2(0, offsetY);
		}
		Vector2 controlPosition = uiControl->GetPosition(true);
		controlPosition += moveDelta;
		uiControl->SetPosition(controlPosition, true);
	}

	return resultData;
}

///////////////////////////////////////////////////////////////////////////////////////////
	
ControlsPositionData DoNothingAlignHandler::Align(const List<UIControl*>& controlsList)
{
	ControlsPositionData resultData;
	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		if (*iter)
		{
			resultData.AddControl(*iter);
		}
	}
	
	return resultData;
}

///////////////////////////////////////////////////////////////////////////////////////////
	
ControlsPositionData AlignLeftHandler::Align(const List<UIControl*>& controlsList)
{
	return AlignLeftTop(controlsList, true);
}
	
///////////////////////////////////////////////////////////////////////////////////////////

ControlsPositionData AlignHorzCenterHandler::Align(const List<UIControl*>& controlsList)
{
	return AlignCenter(controlsList, true);
}

///////////////////////////////////////////////////////////////////////////////////////////

ControlsPositionData AlignRightHandler::Align(const List<UIControl*>& controlsList)
{
	return AlignRightBottom(controlsList, true);
}

///////////////////////////////////////////////////////////////////////////////////////////

ControlsPositionData AlignTopHandler::Align(const List<UIControl*>& controlsList)
{
	return AlignLeftTop(controlsList, false);
}
	
///////////////////////////////////////////////////////////////////////////////////////////

ControlsPositionData AlignVertCenterHandler::Align(const List<UIControl*>& controlsList)
{
	return AlignCenter(controlsList, false);
}

///////////////////////////////////////////////////////////////////////////////////////////

ControlsPositionData AlignBottomHandler::Align(const List<UIControl*>& controlsList)
{
	return AlignRightBottom(controlsList, false);
}

};
