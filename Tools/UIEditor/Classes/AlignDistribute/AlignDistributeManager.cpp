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


#include "AlignDistributeManager.h"

namespace DAVA {
	
ControlsPositionData AlignDistributeManager::AlignControls(const List<UIControl*>& controlsList,
														   eAlignControlsType alignType)
{
	BaseAlignHandler* alignHandler = CreateAlignHandler(alignType);
	DVASSERT(alignHandler);

	ControlsPositionData resutlData = alignHandler->Align(controlsList);
	SafeDelete(alignHandler);
	
	return resutlData;
}

ControlsPositionData AlignDistributeManager::DistributeControls(const List<UIControl*>& controlsList, eDistributeControlsType distributeType)
{
	BaseDistributeHandler* distributeHandler = CreateDistributeHandler(distributeType);
	DVASSERT(distributeHandler);

	ControlsPositionData resutlData = distributeHandler->Distribute(controlsList);
	SafeDelete(distributeHandler);
	
	return resutlData;
}

void AlignDistributeManager::UndoAlignDistribute(const ControlsPositionData& positionData)
{
	for (Map<UIControl*, Rect>::const_iterator iter = positionData.GetControlPositions().begin();
		 iter != positionData.GetControlPositions().end(); iter ++)
	{
		UIControl* control = iter->first;
		Rect rect = iter->second;

		if (control)
		{
			control->SetRect(rect);
		}
	}
}

BaseAlignHandler* AlignDistributeManager::CreateAlignHandler(eAlignControlsType alignType)
{
	switch (alignType)
	{
		case ALIGN_CONTROLS_LEFT:
		{
			return new AlignLeftHandler();
		}
		
		case ALIGN_CONTROLS_HORZ_CENTER:
		{
			return new AlignHorzCenterHandler();
		}
			
		case ALIGN_CONTROLS_RIGHT:
		{
			return new AlignRightHandler();
		}

		case ALIGN_CONTROLS_TOP:
		{
			return new AlignTopHandler();
		}

		case ALIGN_CONTROLS_VERT_CENTER:
		{
			return new AlignVertCenterHandler();
		}

		case ALIGN_CONTROLS_BOTTOM:
		{
			return new AlignBottomHandler();
		}

		default:
		{
			Logger::Error("No Align Handler found for Align Type %i", alignType);
			DVASSERT(false);
			return new DoNothingAlignHandler();
		}
	}
}

BaseDistributeHandler* AlignDistributeManager::CreateDistributeHandler(eDistributeControlsType distributeType)
{
	switch (distributeType)
	{
		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_LEFT_EDGES:
		{
			return new DistributeEqualDistanceLeftEdgesHandler();
		}

		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_X_CENTERS:
		{
			return new DistributeEqualDistanceXCentersHandler();
		}

		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_RIGHT_EDGES:
		{
			return new DistributeEqualDistanceRightEdgesHandler();
		}

		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_X:
		{
			return new DistributeEqualDistanceXHandler();
		}

		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_TOP_EDGES:
		{
			return new DistributeEqualDistanceTopEdgesHandler();
		}

		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_Y_CENTERS:
		{
			return new DistributeEqualDistanceYCentersHandler();
		}

		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_BOTTOM_EDGES:
		{
			return new DistributeEqualDistanceBottomEdgesHandler();
		}

		case DISTRIBUTE_CONTROLS_EQUAL_DISTANCE_BETWEEN_Y:
		{
			return new DistributeEqualDistanceYHandler();
		}

		default:
		{
			Logger::Error("No Distribute Handler found for Distribute Type %i", distributeType);
			DVASSERT(false);
			return new DoNothingDistributeHandler();
		}
	}
}

};