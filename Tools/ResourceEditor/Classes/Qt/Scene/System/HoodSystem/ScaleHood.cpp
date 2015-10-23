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


#include "Scene/System/HoodSystem/ScaleHood.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/TextDrawSystem.h"

// framework
#include "Render/RenderHelper.h"


ScaleHood::ScaleHood() : HoodObject(4.0f)
	, modifScale(0)
{
	DAVA::float32 c = 2 * baseSize / 3;

	axisX = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = ST_AXIS_X;

	axisY = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = ST_AXIS_Y;

	axisZ = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = ST_AXIS_Z;

	axisXY = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(0, c, 0));
	axisXY->axis = ST_AXIS_XY;

	axisXZ = CreateLine(DAVA::Vector3(c, 0, 0), DAVA::Vector3(0, 0, c));
	axisXZ->axis = ST_AXIS_XZ;

	axisYZ = CreateLine(DAVA::Vector3(0, c, 0), DAVA::Vector3(0, 0, c));
	axisYZ->axis = ST_AXIS_YZ;
}

ScaleHood::~ScaleHood()
{

}

void ScaleHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper * drawer, TextDrawSystem *textDrawSystem)
{
	// x
	if(mouseOverAxis) 
        drawer->DrawLine(axisX->curFrom, axisX->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	else 
        drawer->DrawLine(axisX->curFrom, axisX->curTo, colorX, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

	// y
	if(mouseOverAxis) 
        drawer->DrawLine(axisY->curFrom, axisY->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	else 
        drawer->DrawLine(axisY->curFrom, axisY->curTo, colorY, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

	// z
	if(mouseOverAxis) 
        drawer->DrawLine(axisZ->curFrom, axisZ->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	else 
        drawer->DrawLine(axisZ->curFrom, axisZ->curTo, colorZ, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

	// xy xz yz axis
    drawer->DrawLine(axisXY->curFrom, axisXY->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
    drawer->DrawLine(axisXZ->curFrom, axisXZ->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
    drawer->DrawLine(axisYZ->curFrom, axisYZ->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

	// xy xz yz plane
	if(mouseOverAxis)
	{
		DAVA::Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);

		DAVA::Polygon3 poly;
		poly.AddPoint(axisXY->curFrom);
		poly.AddPoint(axisXY->curTo);
		poly.AddPoint(axisYZ->curTo);
        drawer->DrawPolygon(poly, colorSBlend, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
	}

	// draw axis spheres
	DAVA::float32 boxSize = axisX->curScale * baseSize / 12;

    drawer->DrawAABox(DAVA::AABBox3(axisX->curTo, boxSize), colorX, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);

    drawer->DrawAABox(DAVA::AABBox3(axisY->curTo, boxSize), colorY, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);

    drawer->DrawAABox(DAVA::AABBox3(axisZ->curTo, boxSize), colorZ, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);

	DAVA::Rect r = DrawAxisText(textDrawSystem, axisX, axisY, axisZ);

	if(0 != modifScale)
	{
		char tmp[255];
		DAVA::Vector2 topPos = DAVA::Vector2((r.x + r.dx)/2, r.y - 20);

		sprintf(tmp, "[%.2f, %.2f, %.2f]", modifScale, modifScale, modifScale);
        textDrawSystem->DrawText(topPos, tmp, DAVA::Color(1.0f, 1.0f, 0.0f, 1.0f));
    }
}
