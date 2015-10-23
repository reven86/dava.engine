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


#include "Scene/System/HoodSystem/RotateHood.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/TextDrawSystem.h"

// framework
#include "Render/RenderHelper.h"

RotateHood::RotateHood() : HoodObject(4.0f)
	, modifRotate(0)
{
	DAVA::float32 b = baseSize / 4;
	radius = 2 * baseSize / 3;

	DAVA::float32 step = DAVA::PI_05 / ROTATE_HOOD_CIRCLE_PARTS_COUNT;
	DAVA::float32 x, y, lx = radius, ly = 0;

	axisX = CreateLine(DAVA::Vector3(b, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = ST_AXIS_X;
	axisY = CreateLine(DAVA::Vector3(0, b, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = ST_AXIS_Y;
	axisZ = CreateLine(DAVA::Vector3(0, 0, b), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = ST_AXIS_Z;

	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
		DAVA::float32 angle = step * (i + 1);

		x = radius * cosf(angle);
		y = radius * sinf(angle);

		axisXc[i] = CreateLine(DAVA::Vector3(0, lx, ly), DAVA::Vector3(0, x, y));
		axisXc[i]->axis = ST_AXIS_X;
		axisYc[i] = CreateLine(DAVA::Vector3(lx, 0, ly), DAVA::Vector3(x, 0, y));
		axisYc[i]->axis = ST_AXIS_Y;
		axisZc[i] = CreateLine(DAVA::Vector3(lx, ly, 0), DAVA::Vector3(x, y, 0));
		axisZc[i]->axis = ST_AXIS_Z;

		lx = x;
		ly = y;
	}	    
}

RotateHood::~RotateHood()
{ }

void RotateHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper * drawer, TextDrawSystem *textDrawSystem)
{
	DAVA::Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
	DAVA::Vector3 curPos = axisX->curPos;

    DAVA::Color lineColor = colorX;

	// x
	if(selectedAxis == ST_AXIS_X || selectedAxis == ST_AXIS_YZ) 
	{
		if(0 == modifRotate)
		{
			DAVA::Polygon3 poly;
			poly.AddPoint(curPos);
			for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
			{
				poly.AddPoint(axisXc[i]->curFrom);
			}
			poly.AddPoint(axisXc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
            drawer->DrawPolygon(poly, colorSBlend, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
		}
		// draw rotate circle
		else
		{
			DAVA::float32 step = modifRotate / 24;
			DAVA::Color modifColor = colorX;
			modifColor.a = 0.3f;

			DAVA::Polygon3 poly;
			DAVA::float32 y;
			DAVA::float32 z;

			poly.AddPoint(curPos);
			for (DAVA::float32 a = 0; fabs(a) < fabs(modifRotate); a += step)
			{
				y = radius * sinf(a) * objScale;
				z = radius * cosf(a) * objScale;

				poly.AddPoint(DAVA::Vector3(curPos.x, curPos.y + y, curPos.z + z));
			}

			y = radius * sinf(modifRotate) * objScale;
			z = radius * cosf(modifRotate) * objScale;
			poly.AddPoint(DAVA::Vector3(curPos.x, curPos.y + y, curPos.z + z));
            drawer->DrawPolygon(poly, modifColor, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
		}

        lineColor = colorS;
	}

    drawer->DrawLine(axisX->curFrom, axisX->curTo, lineColor, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
        drawer->DrawLine(axisXc[i]->curFrom, axisXc[i]->curTo, lineColor, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	}

    lineColor = colorY;
	// y
	if(selectedAxis == ST_AXIS_Y || selectedAxis == ST_AXIS_XZ) 
	{
		if(0 == modifRotate)
		{
			DAVA::Polygon3 poly;
			poly.AddPoint(curPos);
			for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
			{
				poly.AddPoint(axisYc[i]->curFrom);
			}
			poly.AddPoint(axisYc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
            drawer->DrawPolygon(poly, colorSBlend, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
		}
		// draw rotate circle
		else
		{
			DAVA::float32 step = modifRotate / 24;
			DAVA::Color modifColor = colorY;
			modifColor.a = 0.3f;

			DAVA::Polygon3 poly;
			DAVA::float32 x;
			DAVA::float32 z;

			poly.AddPoint(curPos);
			for (DAVA::float32 a = 0; fabs(a) < fabs(modifRotate); a += step)
			{
				x = radius * cosf(a) * objScale;
				z = radius * sinf(a) * objScale;

				poly.AddPoint(DAVA::Vector3(curPos.x + x, curPos.y, curPos.z + z));
			}

			x = radius * cosf(modifRotate) * objScale;
			z = radius * sinf(modifRotate) * objScale;
			poly.AddPoint(DAVA::Vector3(curPos.x + x, curPos.y, curPos.z + z));
            drawer->DrawPolygon(poly, modifColor, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
		}
	
        lineColor = colorS;
	}

    drawer->DrawLine(axisY->curFrom, axisY->curTo, lineColor, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
        drawer->DrawLine(axisYc[i]->curFrom, axisYc[i]->curTo, lineColor, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	}

    lineColor = colorZ;
	// z
	if(selectedAxis == ST_AXIS_Z || selectedAxis == ST_AXIS_XY)
	{
		if(0 == modifRotate)
		{
			DAVA::Polygon3 poly;
			poly.AddPoint(curPos);
			for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
			{
				poly.AddPoint(axisZc[i]->curFrom);
			}
			poly.AddPoint(axisZc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
            drawer->DrawPolygon(poly, colorSBlend, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
		}
		// draw rotate circle
		else
		{
			DAVA::float32 step = modifRotate / 24;
			DAVA::Color modifColor = colorZ;
			modifColor.a = 0.3f;

			DAVA::Polygon3 poly;
			DAVA::float32 x;
			DAVA::float32 y;

			poly.AddPoint(curPos);
			for (DAVA::float32 a = 0; fabs(a) < fabs(modifRotate); a += step)
			{
				x = radius * sinf(a) * objScale;
				y = radius * cosf(a) * objScale;

				poly.AddPoint(DAVA::Vector3(curPos.x + x, curPos.y + y, curPos.z));
			}

			x = radius * sinf(modifRotate) * objScale;
			y = radius * cosf(modifRotate) * objScale;
			poly.AddPoint(DAVA::Vector3(curPos.x + x, curPos.y + y, curPos.z));
            drawer->DrawPolygon(poly, modifColor, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
		}
	
        lineColor = colorS;
	}

    drawer->DrawLine(axisZ->curFrom, axisZ->curTo, lineColor, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	for(int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
	{
        drawer->DrawLine(axisZc[i]->curFrom, axisZc[i]->curTo, lineColor, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
	}

	// draw axis spheres
	DAVA::float32 radius = axisX->curScale * baseSize / 24;

    drawer->DrawIcosahedron(axisX->curTo, radius, colorX, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
    drawer->DrawIcosahedron(axisY->curTo, radius, colorY, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);
    drawer->DrawIcosahedron(axisZ->curTo, radius, colorZ, DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH);

	DAVA::Rect r = DrawAxisText(textDrawSystem, axisX, axisY, axisZ);

	if(0 != modifRotate)
	{
		char tmp[255];
		tmp[0] = 0;

		if(selectedAxis == ST_AXIS_X || selectedAxis == ST_AXIS_YZ)
		{
			sprintf(tmp, "[%.2f, 0.00, 0.00]", DAVA::RadToDeg(modifRotate));
		}
		if(selectedAxis == ST_AXIS_Y || selectedAxis == ST_AXIS_XZ)
		{
			sprintf(tmp, "[0.00, %.2f, 0.00]", DAVA::RadToDeg(modifRotate));
		}
		if(selectedAxis == ST_AXIS_Z || selectedAxis == ST_AXIS_XY)
		{
			sprintf(tmp, "[0.00, 0.00, %.2f]", DAVA::RadToDeg(modifRotate));
		}

		if(0 != tmp[0])
		{
			DAVA::Vector2 topPos = DAVA::Vector2((r.x + r.dx)/2, r.y - 20);
            textDrawSystem->DrawText(topPos, tmp, DAVA::Color(1.0f, 1.0f, 0.0f, 1.0f));
        }
	}
}
