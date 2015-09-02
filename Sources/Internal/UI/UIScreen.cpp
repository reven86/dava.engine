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


#include "UI/UIScreen.h"
#include "Render/RenderHelper.h"
#include "Platform/SystemTimer.h"
#include <Render/2D/Systems/RenderSystem2D.h>

namespace DAVA 
{

List<UIScreen*> UIScreen::appScreens;
int32			UIScreen::groupIdCounter = -1;

UIScreen::UIScreen(const Rect &rect)
	:UIControl(rect)
,	groupId(groupIdCounter)
{
	// add screen to list
	appScreens.push_back(this);
	groupIdCounter --;
	isLoaded = false;
	fillBorderOrder = FILL_BORDER_AFTER_DRAW;
    fullScreenRect = rect;
}
	
UIScreen::~UIScreen()
{
	// remove screen from list
	for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
	{
		if (*t == this)
		{
			appScreens.erase(t);
			break;
		}
	}
}
    
void UIScreen::SystemWillAppear()
{
    UIControl::SystemWillAppear();

    if (fullScreenRect.dx != VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect().dx
        || fullScreenRect.dy != VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect().dy)
    {
        SystemScreenSizeDidChanged(VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect());
    }
}

void UIScreen::SystemScreenSizeDidChanged(const Rect &newFullScreenRect)
{
    fullScreenRect = newFullScreenRect;
    UIControl::SystemScreenSizeDidChanged(newFullScreenRect);
}

	
void UIScreen::SetFillBorderOrder(UIScreen::eFillBorderOrder fillOrder)
{
	fillBorderOrder = fillOrder;
}

	
void UIScreen::SystemDraw(const UIGeometricData &geometricData)
{
	if (fillBorderOrder == FILL_BORDER_BEFORE_DRAW)
	{
		FillScreenBorders(geometricData);
		UIControl::SystemDraw(geometricData);
	}
	else if (fillBorderOrder == FILL_BORDER_AFTER_DRAW)
	{
		UIControl::SystemDraw(geometricData);
		FillScreenBorders(geometricData);
	}
	else 
	{
		UIControl::SystemDraw(geometricData);
	}
}

void UIScreen::FillScreenBorders(const UIGeometricData &geometricData)
{
    static auto drawColor(Color::Black);

	UIGeometricData drawData;
	drawData.position = relativePosition;
	drawData.size = size;
	drawData.pivotPoint = GetPivotPoint();
	drawData.scale = scale;
	drawData.angle = angle;
    drawData.AddGeometricData(geometricData);

	Rect drawRect = drawData.GetUnrotatedRect();
    Rect fullRect = VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect();
    Vector2 virtualSize = Vector2((float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx,
                                  (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy);
	if (fullRect.x < 0)
	{
	    auto rect1 = Rect(fullRect.x, 0, -fullRect.x, virtualSize.y);
        RenderSystem2D::Instance()->FillRect(rect1, drawColor);
	    auto rect2 = Rect(fullRect.dx - fullRect.x, 0, fullRect.x, virtualSize.y);
        RenderSystem2D::Instance()->FillRect(rect2, drawColor);
	}
	else 
	{
	    auto rect1 = Rect(0, fullRect.y, virtualSize.x + 1,	-fullRect.y);
        RenderSystem2D::Instance()->FillRect(rect1, drawColor);
        auto rect2 = Rect(0, fullRect.dy, virtualSize.x + 1, -fullRect.y);
        RenderSystem2D::Instance()->FillRect(rect2, drawColor);
	}
}


void UIScreen::LoadGroup()
{
	//Logger::FrameworkDebug("load group started");
	//uint64 loadGroupStart = SystemTimer::Instance()->AbsoluteMS();
	if (groupId < 0)
	{
		if (isLoaded)return;
		
		LoadResources();
		isLoaded = true;
	}else
	{
		int32 screenGroupId = groupId;
		for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
		{
			UIScreen * screen = *t;
			if ((screen->groupId == screenGroupId) && (!screen->isLoaded))
			{
				screen->LoadResources();
				screen->isLoaded = true;
			}
		}
	}
	//uint64 loadGroupEnd = SystemTimer::Instance()->AbsoluteMS();
	//Logger::FrameworkDebug("load group finished: %lld", loadGroupEnd - loadGroupStart);
}

void UIScreen::UnloadGroup()
{
	if (groupId < 0)
	{
		if (!isLoaded)return;
		
		UnloadResources();
		isLoaded = false;
	}else
	{
		int32 screenGroupId = groupId;
		for (List<UIScreen*>::iterator t = appScreens.begin(); t != appScreens.end(); ++t)
		{
			UIScreen * screen = *t;
			if ((screen->groupId == screenGroupId) && (screen->isLoaded))
			{
				screen->UnloadResources();
				screen->isLoaded = false;
			}
		}
	}	
}

bool UIScreen::IsLoaded()
{
	return isLoaded;
}
	
void UIScreen::AddToGroup(int32 _groupId)
{
	groupId = _groupId;
}

void UIScreen::RemoveFromGroup()
{
	groupId = groupIdCounter--;
}

int32 UIScreen::GetGroupId()
{
	return groupId;
}

};