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



#include "UI/UIMoveInTransition.h"
#include "Render/RenderManager.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"

namespace DAVA 
{

	
UIMoveInTransition::UIMoveInTransition()
{
	type = FROM_TOP;
	isOver = false;
}

UIMoveInTransition::~UIMoveInTransition()
{
}
	
void UIMoveInTransition::SetType(eType _type, bool moveOver)
{
	type = _type;
	isOver = moveOver;
}

void UIMoveInTransition::Update(float32 timeElapsed)
{
	UIScreenTransition::Update(timeElapsed);
}

void UIMoveInTransition::Draw(const UIGeometricData &geometricData)
{
	/*
	 renderTargetPrevScreen->SetScale(0.5f, 1.0f);
	 renderTargetPrevScreen->SetPosition(0, 0);
	 renderTargetPrevScreen->Draw();

	 renderTargetNextScreen->SetScale(0.5f, 1.0f);
	 renderTargetNextScreen->SetPosition(240, 0);
	 renderTargetNextScreen->Draw(); 

	 FROM_LEFT = 0, 
	 FROM_RIGHT,
	 FROM_TOP,
	 FROM_BOTTOM,
	 */
	
    Sprite::DrawState drawState;
    drawState.SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
    
	if(type <= FROM_BOTTOM)
	{
		float32 endXPos[4] = {(Core::Instance()->GetVirtualScreenXMax() - Core::Instance()->GetVirtualScreenXMin()), -(Core::Instance()->GetVirtualScreenXMax() - Core::Instance()->GetVirtualScreenXMin()), 0.0f, 0.0f};
		float32 endYPos[4] = {0.0f, 0.0f, (Core::Instance()->GetVirtualScreenYMax() - Core::Instance()->GetVirtualScreenYMin()), -(Core::Instance()->GetVirtualScreenYMax() - Core::Instance()->GetVirtualScreenYMin())};
		float32 xPrevPosition = endXPos[type] * normalizedTime;
		float32 yPrevPosition = endYPos[type] * normalizedTime;
		float32 xNextPosition = xPrevPosition - endXPos[type];
		float32 yNextPosition = yPrevPosition - endYPos[type];
		
		if(!isOver)
		{
            drawState.SetPosition(xPrevPosition, yPrevPosition);
		}
		else 
		{
			drawState.SetPosition(0, 0);
		}
        
		renderTargetPrevScreen->Draw(&drawState);
		
		
		drawState.SetPosition(xNextPosition, yNextPosition);
		renderTargetNextScreen->Draw(&drawState);
	}
	else 
	{
		float32 endXPos[4] = {(Core::Instance()->GetVirtualScreenXMax() - Core::Instance()->GetVirtualScreenXMin()), -(Core::Instance()->GetVirtualScreenXMax() - Core::Instance()->GetVirtualScreenXMin()), 0.0f, 0.0f};
		float32 endYPos[4] = {0.0f, 0.0f, (Core::Instance()->GetVirtualScreenYMax() - Core::Instance()->GetVirtualScreenYMin()), -(Core::Instance()->GetVirtualScreenYMax() - Core::Instance()->GetVirtualScreenYMin())};
		float32 xPrevPosition = endXPos[type - 4] * normalizedTime;
		float32 yPrevPosition = endYPos[type - 4] * normalizedTime;
		float32 xNextPosition = xPrevPosition - endXPos[type - 4];
		float32 yNextPosition = yPrevPosition - endYPos[type - 4];
		
		if(!isOver)
		{
			drawState.SetPosition(xNextPosition, yNextPosition);
		}
		else 
		{
			drawState.SetPosition(0, 0);
		}
        
		renderTargetNextScreen->Draw(&drawState);

		
		drawState.SetPosition(xPrevPosition, yPrevPosition);
		renderTargetPrevScreen->Draw(&drawState);
		
	}


}
	
};

