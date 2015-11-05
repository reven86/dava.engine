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


#include "UI/UIFadeTransition.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/Renderer.h"
#include "Render/RenderHelper.h"

namespace DAVA 
{

	
UIFadeTransition::UIFadeTransition()
{
	type = FADE_MIX;
}

UIFadeTransition::~UIFadeTransition()
{
}
	
void UIFadeTransition::SetType(eType _type)
{
	type = _type;
}

void UIFadeTransition::Update(float32 timeElapsed)
{
	UIScreenTransition::Update(timeElapsed);
}

void UIFadeTransition::Draw(const UIGeometricData &geometricData)
{
	/*
	 renderTargetPrevScreen->SetScale(0.5f, 1.0f);
	 renderTargetPrevScreen->SetPosition(0, 0);
	 renderTargetPrevScreen->Draw();

	 renderTargetNextScreen->SetScale(0.5f, 1.0f);
	 renderTargetNextScreen->SetPosition(240, 0);
	 renderTargetNextScreen->Draw(); 
	 */
    Sprite::DrawState drawState;
    drawState.SetMaterial(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);
    if (type == FADE_MIX)
    {
        renderTargetPrevScreen->Reset();
        drawState.SetPosition(geometricData.position);
        RenderSystem2D::Instance()->Draw(renderTargetPrevScreen, &drawState, Color::White);

        renderTargetNextScreen->Reset();
        drawState.SetPosition(geometricData.position);
        RenderSystem2D::Instance()->Draw(renderTargetNextScreen, &drawState, Color(1.0f, 1.0f, 1.0f, normalizedTime));
    }
    else if (type == FADE_IN_FADE_OUT)
	{
		if (normalizedTime <= 0.5f)
		{
			drawState.SetPosition(0, 0);
            RenderSystem2D::Instance()->Draw(renderTargetPrevScreen, &drawState, Color(1.0f - normalizedTime * 2, 1.0f - normalizedTime * 2, 1.0f - normalizedTime * 2, 1.0f));
        }
        else
		{
			drawState.SetPosition(0, 0);
            RenderSystem2D::Instance()->Draw(renderTargetNextScreen, &drawState, Color((normalizedTime - 0.5f) * 2, (normalizedTime - 0.5f) * 2, (normalizedTime - 0.5f) * 2, 1.0f));
        }
    }
}
	
};

