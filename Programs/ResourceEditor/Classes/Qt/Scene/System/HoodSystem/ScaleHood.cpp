#include "Scene/System/HoodSystem/ScaleHood.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/TextDrawSystem.h"

// framework
#include "Render/RenderHelper.h"

ScaleHood::ScaleHood()
    : HoodObject(4.0f)
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

void ScaleHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper* drawer, TextDrawSystem* textDrawSystem)
{
    // x
    if (mouseOverAxis)
        drawer->DrawLine(axisX->curFrom, axisX->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisX->curFrom, axisX->curTo, colorX, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

    // y
    if (mouseOverAxis)
        drawer->DrawLine(axisY->curFrom, axisY->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisY->curFrom, axisY->curTo, colorY, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

    // z
    if (mouseOverAxis)
        drawer->DrawLine(axisZ->curFrom, axisZ->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisZ->curFrom, axisZ->curTo, colorZ, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

    // xy xz yz axis
    drawer->DrawLine(axisXY->curFrom, axisXY->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
    drawer->DrawLine(axisXZ->curFrom, axisXZ->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);
    drawer->DrawLine(axisYZ->curFrom, axisYZ->curTo, colorS, DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH);

    // xy xz yz plane
    if (mouseOverAxis)
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

    if (0 != modifScale)
    {
        char tmp[255];
        DAVA::Vector2 topPos = DAVA::Vector2((r.x + r.dx) / 2, r.y - 20);

        sprintf(tmp, "[%.2f, %.2f, %.2f]", modifScale, modifScale, modifScale);
        textDrawSystem->DrawText(topPos, tmp, DAVA::Color(1.0f, 1.0f, 0.0f, 1.0f));
    }
}
