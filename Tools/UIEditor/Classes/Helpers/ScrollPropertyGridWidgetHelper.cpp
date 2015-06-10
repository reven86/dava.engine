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


#include "ScrollPropertyGridWidgetHelper.h"

using namespace DAVA;

const ScrollPropertyGridWidgetHelper::OrientationData ScrollPropertyGridWidgetHelper::orientationData[] =
{
    {UIScrollBar::ORIENTATION_VERTICAL,           "Vertical"},
    {UIScrollBar::ORIENTATION_HORIZONTAL,   		"Horizontal"}
};

// Get the scroll of UIControlStates supported:
int ScrollPropertyGridWidgetHelper::GetOrientationCount()
{
    return sizeof(orientationData) / sizeof(*orientationData);
}

UIScrollBar::eScrollOrientation ScrollPropertyGridWidgetHelper::GetOrientation(int index)
{
	if (ValidateOrientationIndex(index) == false)
	{
		return orientationData[0].orientation;
	}
	
	return orientationData[index].orientation;
}

QString ScrollPropertyGridWidgetHelper::GetOrientationDesc(int index)
{
    if (ValidateOrientationIndex(index) == false)
    {
        return orientationData[0].orientationDesc;
    }
    
    return orientationData[index].orientationDesc;
}

QString ScrollPropertyGridWidgetHelper::GetOrientationDescByType(UIScrollBar::eScrollOrientation orientation)
{
	int count = GetOrientationCount();
	for (int i = 0; i < count; i++)
	{
		if (orientation == orientationData[i].orientation)
		{
			return orientationData[i].orientationDesc;
		}
	}
	
	Logger::Error("Unknown/unsupported Orientation Type %i!", orientation);
    return QString();
}

bool ScrollPropertyGridWidgetHelper::ValidateOrientationIndex(int index)
{
    if (index < 0 || index >= GetOrientationCount())
    {
        Logger::Error("Orientation index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}
