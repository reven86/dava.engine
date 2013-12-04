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



#include "Scene/System/HoodSystem/NormalHood.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/TextDrawSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

NormalHood::NormalHood() : HoodObject(2.0f)
{
	axisX = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = ST_AXIS_X;

	axisY = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = ST_AXIS_Y;

	axisZ = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = ST_AXIS_Z;
}

NormalHood::~NormalHood()
{

}

void NormalHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, TextDrawSystem *textDrawSystem)
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
	DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE);
	DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

	// x
	DAVA::RenderManager::Instance()->SetColor(colorX);
	DAVA::RenderHelper::Instance()->DrawLine(axisX->curFrom, axisX->curTo);

	// y
	DAVA::RenderManager::Instance()->SetColor(colorY);
	DAVA::RenderHelper::Instance()->DrawLine(axisY->curFrom, axisY->curTo);

	// z
	DAVA::RenderManager::Instance()->SetColor(colorZ);
	DAVA::RenderHelper::Instance()->DrawLine(axisZ->curFrom, axisZ->curTo);

	DrawAxisText(textDrawSystem, axisX, axisY, axisZ);

	DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	DAVA::RenderManager::Instance()->SetState(oldState);
}
