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


#include "Scene/System/HoodSystem/MoveHood.h"
#include "Scene/System/ModifSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Scene/System/TextDrawSystem.h"

MoveHood::MoveHood() : HoodObject(4.0f)
{
	DAVA::float32 b = baseSize / 5;
	DAVA::float32 c = baseSize / 3;

	axisX = CreateLine(DAVA::Vector3(b, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = ST_AXIS_X;

	axisY = CreateLine(DAVA::Vector3(0, b, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = ST_AXIS_Y;

	axisZ = CreateLine(DAVA::Vector3(0, 0, b), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = ST_AXIS_Z;

	axisXY1 = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(c, c, 0));
	axisXY1->axis = ST_AXIS_XY;

	axisXY2 = CreateLine(DAVA::Vector3(0, c, 0), DAVA::Vector3(c, c, 0));
	axisXY2->axis = ST_AXIS_XY;

	axisXZ1 = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(c, 0, c));
	axisXZ1->axis = ST_AXIS_XZ;

	axisXZ2 = CreateLine(DAVA::Vector3(0, 0, c), DAVA::Vector3(c, 0, c));
	axisXZ2->axis = ST_AXIS_XZ;

	axisYZ1 = CreateLine(DAVA::Vector3(0, c, 0), DAVA::Vector3(0, c, c));
	axisYZ1->axis = ST_AXIS_YZ;
	
	axisYZ2 = CreateLine(DAVA::Vector3(0, 0, c), DAVA::Vector3(0, c, c));
	axisYZ2->axis = ST_AXIS_YZ;
	
    DAVA::RenderStateData hoodStateData;
    DAVA::RenderManager::Instance()->GetRenderStateData(DAVA::RenderState::RENDERSTATE_3D_BLEND, hoodStateData);
  	
	hoodStateData.state =	DAVA::RenderStateData::STATE_BLEND |
							DAVA::RenderStateData::STATE_COLORMASK_ALL |
							DAVA::RenderStateData::STATE_DEPTH_WRITE;
	hoodStateData.sourceFactor = DAVA::BLEND_SRC_ALPHA;
	hoodStateData.destFactor = DAVA::BLEND_ONE_MINUS_SRC_ALPHA;
	hoodDrawState = DAVA::RenderManager::Instance()->CreateRenderState(hoodStateData);
}

MoveHood::~MoveHood()
{

}

void MoveHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, TextDrawSystem *textDrawSystem)
{
	DAVA::Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
	DAVA::Vector3 curPos = axisX->curPos;

	// x
	if(selectedAxis & ST_AXIS_X) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorX);

	DAVA::RenderHelper::Instance()->DrawLine(axisX->curFrom, axisX->curTo, 1.0f, hoodDrawState);
	
	// y
	if(selectedAxis & ST_AXIS_Y) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorY);

	DAVA::RenderHelper::Instance()->DrawLine(axisY->curFrom, axisY->curTo, 1.0f, hoodDrawState);

	// z
	if(selectedAxis & ST_AXIS_Z) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorZ);

	DAVA::RenderHelper::Instance()->DrawLine(axisZ->curFrom, axisZ->curTo, 1.0f, hoodDrawState);

	// arrow length
	DAVA::float32 arrowLen = axisX->curScale * baseSize / 4;

	// arrow x
	DAVA::RenderManager::Instance()->SetColor(colorX);
	DAVA::RenderHelper::Instance()->FillArrow(axisX->curFrom, axisX->curTo, arrowLen, 0, hoodDrawState);

	// arrow y
	DAVA::RenderManager::Instance()->SetColor(colorY);
	DAVA::RenderHelper::Instance()->FillArrow(axisY->curFrom, axisY->curTo, arrowLen, 0, hoodDrawState);

	// arrow z
	DAVA::RenderManager::Instance()->SetColor(colorZ);
	DAVA::RenderHelper::Instance()->FillArrow(axisZ->curFrom, axisZ->curTo, arrowLen, 0, hoodDrawState);


	// xy
	if(selectedAxis == ST_AXIS_XY) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorS);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY1->curFrom, axisXY1->curTo, 1.0f, hoodDrawState);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY2->curFrom, axisXY2->curTo, 1.0f, hoodDrawState);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		poly.AddPoint(axisXY1->curFrom);
		poly.AddPoint(axisXY1->curTo);
		poly.AddPoint(axisXY2->curFrom);
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);
		DAVA::RenderHelper::Instance()->FillPolygon(poly, hoodDrawState);
	}
	else 
	{
		DAVA::RenderManager::Instance()->SetColor(colorX);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY1->curFrom, axisXY1->curTo, 1.0f, hoodDrawState);
		DAVA::RenderManager::Instance()->SetColor(colorY);
		DAVA::RenderHelper::Instance()->DrawLine(axisXY2->curFrom, axisXY2->curTo, 1.0f, hoodDrawState);
	}

	// xz
	if(selectedAxis == ST_AXIS_XZ) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorS);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ1->curFrom, axisXZ1->curTo, 1.0f, hoodDrawState);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ2->curFrom, axisXZ2->curTo, 1.0f, hoodDrawState);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		poly.AddPoint(axisXZ1->curFrom);
		poly.AddPoint(axisXZ1->curTo);
		poly.AddPoint(axisXZ2->curFrom);
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);
		DAVA::RenderHelper::Instance()->FillPolygon(poly, hoodDrawState);
	}
	else 
	{
		DAVA::RenderManager::Instance()->SetColor(colorX);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ1->curFrom, axisXZ1->curTo, 1.0f, hoodDrawState);
		DAVA::RenderManager::Instance()->SetColor(colorZ);
		DAVA::RenderHelper::Instance()->DrawLine(axisXZ2->curFrom, axisXZ2->curTo, 1.0f, hoodDrawState);
	}

	// yz
	if(selectedAxis == ST_AXIS_YZ) 
	{
		DAVA::RenderManager::Instance()->SetColor(colorS);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ1->curFrom, axisYZ1->curTo, 1.0f, hoodDrawState);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ2->curFrom, axisYZ2->curTo, 1.0f, hoodDrawState);

		DAVA::Polygon3 poly;
		poly.AddPoint(curPos);
		poly.AddPoint(axisYZ1->curFrom);
		poly.AddPoint(axisYZ1->curTo);
		poly.AddPoint(axisYZ2->curFrom);
		DAVA::RenderManager::Instance()->SetColor(colorSBlend);
		DAVA::RenderHelper::Instance()->FillPolygon(poly, hoodDrawState);
	}
	else 
	{
		DAVA::RenderManager::Instance()->SetColor(colorY);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ1->curFrom, axisYZ1->curTo, 1.0f, hoodDrawState);
		DAVA::RenderManager::Instance()->SetColor(colorZ);
		DAVA::RenderHelper::Instance()->DrawLine(axisYZ2->curFrom, axisYZ2->curTo, 1.0f, hoodDrawState);
	}

	DAVA::Rect r = DrawAxisText(textDrawSystem, axisX, axisY, axisZ);

	if(!modifOffset.IsZero())
	{
		char tmp[255];
		DAVA::Vector2 topPos = DAVA::Vector2((r.x + r.dx)/2, r.y - 20);

		sprintf(tmp, "[%.2f, %.2f, %.2f]", modifOffset.x, modifOffset.y, modifOffset.z);
		textDrawSystem->DrawText(topPos, tmp, DAVA::Color(255, 255, 0, 255));
	}

	//DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	//DAVA::RenderManager::Instance()->SetState(oldState);
}
