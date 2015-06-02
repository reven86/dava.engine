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


#include "DistributeHandlers.h"

namespace DAVA {

ControlsPositionData BaseDistributeHandler::BuildPositionData(const List<UIControl*>& controlsList)
{
	ControlsPositionData resultData;

	for (List<UIControl*>::const_iterator iter = controlsList.begin(); iter != controlsList.end(); iter ++)
	{
		UIControl* uiControl = *iter;
		if (uiControl)
		{
			resultData.AddControl(uiControl);
		}
	}

	return resultData;
}

bool BaseDistributeHandler::SortByXPosition(UIControl* control1, UIControl* control2)
{
	return (control1->GetRect(true).x < control2->GetRect(true).x);
}

bool BaseDistributeHandler::SortByYPosition(UIControl* control1, UIControl* control2)
{
	return (control1->GetRect(true).y < control2->GetRect(true).y);
}

ControlsPositionData BaseDistributeHandler::OrderControlsList(List<UIControl*>& orderedControlsList, bool isHorizontal, bool& canProcess)
{
	// Order the controls according to their coords.
	if (isHorizontal)
	{
		orderedControlsList.sort(SortByXPosition);
	}
	else
	{
		orderedControlsList.sort(SortByYPosition);
	}

	// Distribution is possible only in case if more than 2 controls are selected.
	ControlsPositionData resultData = BuildPositionData(orderedControlsList);
	canProcess = resultData.GetControlPositions().size() > 2;

	return resultData;
}

ControlsPositionData BaseDistributeHandler::DistributeLeftTopEdges(const List<UIControl*>& controlsList,
																   bool isHorizontal)
{
	bool canProcess = false;
	List<UIControl*> orderedControlsList = controlsList;
	ControlsPositionData resultData = OrderControlsList(orderedControlsList, isHorizontal, canProcess);
	if (!canProcess)
	{
		return resultData;
	}

	UIControl* firstControl = orderedControlsList.front();
	UIControl* lastControl = orderedControlsList.back();
	if (!firstControl || ! lastControl)
	{
		return resultData;
	}

	// Calculate the positions/steps needed.
	int32 itemsCount = resultData.GetItemsCount();
	float32 startPos = 0.0f;
	float32 step = 0.0f;
	if (isHorizontal)
	{
		startPos = firstControl->GetGeometricData().GetAABBox().x;
		step = (lastControl->GetGeometricData().GetAABBox().x - firstControl->GetGeometricData().GetAABBox().x) / (itemsCount - 1);
	}
	else
	{
		startPos = firstControl->GetGeometricData().GetAABBox().y;
		step = (lastControl->GetGeometricData().GetAABBox().y - firstControl->GetGeometricData().GetAABBox().y) / (itemsCount - 1);
	}

	// Update the controls.
	int32 index = 0;
	for (List<UIControl*>::const_iterator iter = orderedControlsList.begin(); iter != orderedControlsList.end();
		 iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}
		
		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		Vector2 moveDelta;
		if (isHorizontal)
		{
			float32 newPosX = startPos + step * index;
			moveDelta = Vector2(newPosX - absoluteRect.x, 0);
		}
		else
		{
			float32 newPosY = startPos + step * index;
			moveDelta = Vector2(0, newPosY - absoluteRect.y);
		}

		index ++;
		Vector2 controlPosition = uiControl->GetPosition(true);
		controlPosition += moveDelta;
		uiControl->SetPosition(controlPosition, true);
	}

	return resultData;
}

ControlsPositionData BaseDistributeHandler::DistributeXYCenter(const List<UIControl*>& controlsList, bool isHorizontal)
{
	bool canProcess = false;
	List<UIControl*> orderedControlsList = controlsList;
	ControlsPositionData resultData = OrderControlsList(orderedControlsList, isHorizontal, canProcess);
	if (!canProcess)
	{
		return resultData;
	}

	// Calculate the positions and offsets.
	UIControl* firstControl = orderedControlsList.front();
	UIControl* lastControl = orderedControlsList.back();

	int32 itemsCount = resultData.GetItemsCount();
	float32 startPos = 0.0f;
	float32 step = 0.0f;
	
	if (isHorizontal)
	{
		startPos = firstControl->GetGeometricData().GetAABBox().GetCenter().x;
		step = (lastControl->GetGeometricData().GetAABBox().GetCenter().x - firstControl->GetGeometricData().GetAABBox().GetCenter().x) / (itemsCount - 1);
	}
	else
	{
		startPos = firstControl->GetGeometricData().GetAABBox().GetCenter().y;
		step = (lastControl->GetGeometricData().GetAABBox().GetCenter().y - firstControl->GetGeometricData().GetAABBox().GetCenter().y) / (itemsCount - 1);
	}
	
	// Update the controls.
	int32 index = 0;
	for (List<UIControl*>::const_iterator iter = orderedControlsList.begin(); iter != orderedControlsList.end();
		 iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		Vector2 moveDelta;
		if (isHorizontal)
		{
			float32 newPosX = startPos + step * index - absoluteRect.dx / 2;
			moveDelta = Vector2(newPosX - absoluteRect.x, 0);
		}
		else
		{
			float32 newPosY = startPos + step * index - absoluteRect.dy / 2;;
			moveDelta = Vector2(0, newPosY - absoluteRect.y);
		}

		index ++;
		Vector2 controlPosition = uiControl->GetPosition(true);
		controlPosition += moveDelta;
		uiControl->SetPosition(controlPosition, true);
	}
		
	return resultData;
}

ControlsPositionData BaseDistributeHandler::DistributeRightBottomEdges(const List<UIControl*>& controlsList, bool isHorizontal)
{
	bool canProcess = false;
	List<UIControl*> orderedControlsList = controlsList;
	ControlsPositionData resultData = OrderControlsList(orderedControlsList, isHorizontal, canProcess);
	if (!canProcess)
	{
		return resultData;
	}
	
	// Calculate the positions and offsets.
	UIControl* firstControl = orderedControlsList.front();
	UIControl* lastControl = orderedControlsList.back();

	Rect firstControlRect = firstControl->GetGeometricData().GetAABBox();
	Rect lastControlRect = lastControl->GetGeometricData().GetAABBox();

	float32 startPos = 0.0f;
	float32 endPos = 0.0f;

	if (isHorizontal)
	{
		startPos = firstControlRect.x + firstControlRect.dx;
		endPos = lastControlRect.x + lastControlRect.dx;
	}
	else
	{
		startPos = firstControlRect.y + firstControlRect.dy;
		endPos = lastControlRect.y + lastControlRect.dy;
	}

	int32 itemsCount = resultData.GetItemsCount();
	float32 step = (endPos - startPos) / (itemsCount - 1);

	// Update the controls.
	int32 index = 0;
	for (List<UIControl*>::const_iterator iter = orderedControlsList.begin(); iter != orderedControlsList.end();
		 iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		Vector2 moveDelta;
		if (isHorizontal)
		{
			float32 newPosX = startPos + step * index - absoluteRect.dx;
			moveDelta = Vector2(newPosX - absoluteRect.x, 0);
		}
		else
		{
			float32 newPosY = startPos + step * index - absoluteRect.dy;
			moveDelta = Vector2(0, newPosY - absoluteRect.y);
		}

		index ++;
		Vector2 controlPosition = uiControl->GetPosition(true);
		controlPosition += moveDelta;
		uiControl->SetPosition(controlPosition, true);
	}

	return resultData;
}

ControlsPositionData BaseDistributeHandler::DistributeXY(const List<UIControl*>& controlsList, bool isHorizontal)
{
	bool canProcess = false;
	List<UIControl*> orderedControlsList = controlsList;
	ControlsPositionData resultData = OrderControlsList(orderedControlsList, isHorizontal, canProcess);
	if (!canProcess)
	{
		return resultData;
	}
	
	// Calculate the distance between controls.
	List<UIControl*>::iterator prevControlIter = orderedControlsList.begin();
	List<UIControl*>::iterator secondControlIter = orderedControlsList.begin();
	secondControlIter ++;

	// Calculate the free place between the controls.
	float32 distance = 0.0f;
	for (List<UIControl*>::iterator iter = secondControlIter; iter != orderedControlsList.end();
		 iter ++)
	{
		if (!*iter)
		{
			continue;
		}
			
		Rect prevRect = (*prevControlIter)->GetGeometricData().GetAABBox();
		Rect curRect = (*iter)->GetGeometricData().GetAABBox();
		
		if (isHorizontal)
		{
			distance += (curRect.x - (prevRect.x + prevRect.dx));
		}
		else
		{
			distance += (curRect.y - (prevRect.y + prevRect.dy));
		}

		prevControlIter = iter;
	}

	// Yuri Coder, 2013/10/04. Distance < 0 means that there is no enough place to distribute the controls
	// Corel works in the same way, checked with Olga.
	if (distance <= 0.0f)
	{
		return resultData;
	}

	int32 itemsCount = resultData.GetItemsCount();
	distance /= (itemsCount - 1);

	// Update the controls.
	UIControl* firstControl = orderedControlsList.front();
	Rect firstControlRect = firstControl->GetGeometricData().GetAABBox();
	float32 curPos = isHorizontal ? firstControlRect.x : firstControlRect.y;
	for (List<UIControl*>::iterator iter = orderedControlsList.begin(); iter != orderedControlsList.end();
		 iter ++)
	{
		UIControl* uiControl = *iter;
		if (!uiControl)
		{
			continue;
		}

		Rect absoluteRect = uiControl->GetGeometricData().GetAABBox();
		
		Vector2 moveDelta;
		if (isHorizontal)
		{
			float32 newPosX = curPos;
			moveDelta = Vector2(newPosX - absoluteRect.x, 0);
			curPos += (distance + absoluteRect.dx);
		}
		else
		{
			float32 newPosY = curPos;
			moveDelta = Vector2(0, newPosY - absoluteRect.y);
			curPos += (distance + absoluteRect.dy);
		}

		Vector2 controlPosition = uiControl->GetPosition(true);
		controlPosition += moveDelta;
		uiControl->SetPosition(controlPosition, true);
	}
		
	return resultData;
}

ControlsPositionData DoNothingDistributeHandler::Distribute(const List<UIControl*>& controlsList)
{
	return BuildPositionData(controlsList);
}

ControlsPositionData DistributeEqualDistanceLeftEdgesHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeLeftTopEdges(controlsList, true);
};

ControlsPositionData DistributeEqualDistanceXCentersHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeXYCenter(controlsList, true);
}
	
ControlsPositionData DistributeEqualDistanceRightEdgesHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeRightBottomEdges(controlsList, true);
};

ControlsPositionData DistributeEqualDistanceXHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeXY(controlsList, true);
}

ControlsPositionData DistributeEqualDistanceTopEdgesHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeLeftTopEdges(controlsList, false);
};

ControlsPositionData DistributeEqualDistanceYCentersHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeXYCenter(controlsList, false);
}

ControlsPositionData DistributeEqualDistanceBottomEdgesHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeRightBottomEdges(controlsList, false);
};

ControlsPositionData DistributeEqualDistanceYHandler::Distribute(const List<UIControl*>& controlsList)
{
	return DistributeXY(controlsList, false);
}

};
