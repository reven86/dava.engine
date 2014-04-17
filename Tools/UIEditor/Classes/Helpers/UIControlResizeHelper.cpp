//
//  ResizeHelper.cpp
//  UIEditor
//
//  Created by Yuri Coder on 12/3/13.
//
//

#include "UIControlResizeHelper.h"
#include "Math/MathHelpers.h"

namespace DAVA {

static const float32 MINIMUM_CONTROL_WIDTH = 8.0f;
static const float32 MINIMUM_CONTROL_HEIGHT = 8.0f;

eRotationQuadrant UIControlResizeHelper::GetRotationQuadant(float32 angle)
{
    float32 sinAngle = 0.0f;
    float32 cosAngle = 0.0f;
    SinCosFast(angle, sinAngle, cosAngle);

    static float32 quadrantAngleSinCos = sqrtf(2) / 2; // 45 degrees.
    if ((cosAngle >= 0) && (sinAngle < quadrantAngleSinCos) && (sinAngle > -quadrantAngleSinCos))
    {
        return QUADRANT_MINUS_PI_4_TO_PI_4;
    }
    else if ((sinAngle > 0) && (fabs(cosAngle) < quadrantAngleSinCos))
    {
        return QUADRANT_PI_4_TO_3PI_4;
    }
    else if ((cosAngle < 0) && (sinAngle < quadrantAngleSinCos) && (sinAngle > -quadrantAngleSinCos))
    {
        return QUADRANT_3PI_4_TO_4PI_3;
    }

    return QUADRANT_4PI_3_TO_MINUS_PI_4;
}

    ResizeType UIControlResizeHelper::RotateResizeTypeCounterClockwise(ResizeType originalResizeType)
{
    switch (originalResizeType)
    {
        case ResizeTypeLeft:
            return ResizeTypeBottom;
        
        case ResizeTypeBottom:
            return ResizeTypeRight;

        case ResizeTypeRight:
            return ResizeTypeTop;

        case ResizeTypeTop:
            return ResizeTypeLeft;

        case ResizeTypeLeftBottom:
            return ResizeTypeRightBottom;

        case ResizeTypeRightBottom:
            return ResizeTypeRigthTop;

        case ResizeTypeRigthTop:
            return ResizeTypeLeftTop;

        case ResizeTypeLeftTop:
            return ResizeTypeLeftBottom;
      
        default:
            return originalResizeType;
    }
}
    
ResizeType UIControlResizeHelper::GetRotatedResizeType(ResizeType unrotatedResizeType, UIControl* uiControl)
{
    if (!uiControl)
    {
        return unrotatedResizeType;
    }
    
    // Determine the quadrand and rotete the resize type 0 to 3 times depending on the quadrant.
    eRotationQuadrant quadrant = GetRotationQuadant(uiControl->angle);
    ResizeType rotatedResizeType = unrotatedResizeType;
    for (int32 i = 0; i < (int32)quadrant; i ++)
    {
        rotatedResizeType = RotateResizeTypeCounterClockwise(rotatedResizeType);
    }

    return rotatedResizeType;
}


void UIControlResizeHelper::ResizeControl(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta)
{
    Rect finalResizeRect = GetResizeRect(unrotatedResizeType, uiControl, resizeRect, delta);
    uiControl->SetRect(finalResizeRect);
}

Rect UIControlResizeHelper::GetResizeRect(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta)
{
    if (!uiControl)
    {
        return Rect();
    }

    eRotationQuadrant quadrant = GetRotationQuadant(uiControl->angle);
    switch (quadrant)
    {
        case QUADRANT_MINUS_PI_4_TO_PI_4:
        {
            return ResizeControlInQuadrant0(unrotatedResizeType, uiControl, resizeRect, delta);
        }
        case QUADRANT_PI_4_TO_3PI_4:
        {
            return ResizeControlInQuadrant1(unrotatedResizeType, uiControl, resizeRect, delta);
        }
        case QUADRANT_3PI_4_TO_4PI_3:
        {
            return ResizeControlInQuadrant2(unrotatedResizeType, uiControl, resizeRect, delta);
        }
        case QUADRANT_4PI_3_TO_MINUS_PI_4:
        {
            return ResizeControlInQuadrant3(unrotatedResizeType, uiControl, resizeRect, delta);
        }

        default:
        {
            DVASSERT(false);
            return uiControl->GetRect();
        }
    }
}

float32 UIControlResizeHelper::ClampDX(float32 rawDX)
{
    return (rawDX < MINIMUM_CONTROL_WIDTH) ? MINIMUM_CONTROL_WIDTH : rawDX;
}

float32 UIControlResizeHelper::ClampDY(float32 rawDY)
{
    return (rawDY < MINIMUM_CONTROL_HEIGHT) ? MINIMUM_CONTROL_HEIGHT : rawDY;
}

Rect UIControlResizeHelper::ResizeControlInQuadrant0(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta)
{
    Rect rect = resizeRect;
    const UIGeometricData &gd = uiControl->GetGeometricData();

    // Clamp to minimum width/height.
    float32 offsetX = delta.x;
    float offsetY = delta.y;
    if (rect.dx - offsetX < MINIMUM_CONTROL_HEIGHT)
    {
        offsetX = rect.dx > MINIMUM_CONTROL_HEIGHT ? rect.dx - MINIMUM_CONTROL_HEIGHT : MINIMUM_CONTROL_HEIGHT;
    }
    if (rect.dy - offsetY < MINIMUM_CONTROL_WIDTH)
    {
        offsetY = rect.dx > MINIMUM_CONTROL_WIDTH ? rect.dy - MINIMUM_CONTROL_WIDTH : MINIMUM_CONTROL_WIDTH;
    }

    switch (unrotatedResizeType)
	{
        case ResizeTypeLeft:
        {
            rect.x += offsetX * gd.cosA;
            rect.y += offsetX * gd.sinA;
            rect.dx = ClampDX(rect.dx - offsetX);

            break;
        }

        case ResizeTypeRight:
        {
            rect.dx = ClampDX(rect.dx + delta.x);
            break;
        }

        case ResizeTypeTop:
        {
            rect.x -= (offsetY * gd.sinA);
            rect.y += (offsetY * gd.cosA);
            rect.dy = ClampDY(rect.dy - offsetY);
            break;
        }

        case ResizeTypeBottom:
        {
            rect.dy = ClampDY(rect.dy + offsetY);
            break;
        }

        case ResizeTypeLeftTop:
        {
            rect.x = rect.x + (offsetX * gd.cosA) - (offsetY * gd.sinA);
            rect.y = rect.y + (offsetX * gd.sinA) + (offsetY * gd.cosA);
            rect.dx = ClampDX(rect.dx - offsetX);
            rect.dy = ClampDY(rect.dy - offsetY);

            break;
        }

        case ResizeTypeLeftBottom:
        {
            rect.x += offsetX * gd.cosA;
            rect.y += offsetX * gd.sinA;
            rect.dx = ClampDX(rect.dx - offsetX);
            rect.dy = ClampDY(rect.dy + offsetY);
            break;
        }

        case ResizeTypeRigthTop:
        {
            rect.x -= (offsetY * gd.sinA);
            rect.y += (offsetY * gd.cosA);
            rect.dx = ClampDX(rect.dx + delta.x);
            rect.dy = ClampDY(rect.dy - delta.y);
            break;
        }

        case ResizeTypeRightBottom:
        {
            rect.dx = ClampDX(rect.dx + delta.x);
            rect.dy = ClampDX(rect.dy + delta.y);
            break;
        }

		default:
        {
            break;
        }
    }

    return rect;
}

Rect UIControlResizeHelper::ResizeControlInQuadrant1(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta)
{
    Rect rect = resizeRect;
    const UIGeometricData &gd = uiControl->GetGeometricData();

    float32 offsetX = delta.x;
    float32 offsetY = delta.y;

    switch (unrotatedResizeType)
	{
        case ResizeTypeLeft:
        {
            rect.dx = ClampDX(rect.dx - offsetY);
            if (rect.dx == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = resizeRect.dx - MINIMUM_CONTROL_WIDTH;
            }

            rect.x += offsetY * gd.cosA;
            rect.y += offsetY * gd.sinA;

            break;
        }

        case ResizeTypeTop:
		{
            rect.dy = ClampDY(rect.dy + offsetX);
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = MINIMUM_CONTROL_HEIGHT - resizeRect.dy;
            }

            rect.x += (offsetX * gd.sinA);
            rect.y -= (offsetX * gd.cosA);
            
            break;
        }
 
        case ResizeTypeRight:
        {
            rect.dx = ClampDX(rect.dx + offsetY);
            break;
        }

        case ResizeTypeBottom:
        {
            rect.dy = ClampDY(rect.dy - offsetX);
            break;
        }

        case ResizeTypeLeftTop:
        {
            rect.dx = ClampDX(rect.dx - offsetY);
            rect.dy = ClampDY(rect.dy + offsetX);
            if (rect.dx == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = resizeRect.dx - MINIMUM_CONTROL_WIDTH;
            }
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = MINIMUM_CONTROL_HEIGHT - resizeRect.dy;
            }

            rect.x = rect.x + (offsetY * gd.cosA) + (offsetX * gd.sinA);
            rect.y = rect.y + (offsetY * gd.sinA) - (offsetX * gd.cosA);
            break;
        }

        case ResizeTypeLeftBottom:
        {
            rect.dx = ClampDY(rect.dx - offsetY);
            if (rect.dx == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = resizeRect.dx - MINIMUM_CONTROL_WIDTH;
            }
            
            rect.x += offsetY * gd.cosA;
            rect.y += offsetY * gd.sinA;
            rect.dy = ClampDY(rect.dy - offsetX);
            break;
        }

        case ResizeTypeRigthTop:
        {
            rect.dy = ClampDY(rect.dy + offsetX);
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = MINIMUM_CONTROL_HEIGHT - resizeRect.dy;
            }
            
            rect.x += (offsetX * gd.sinA);
            rect.y -= (offsetX * gd.cosA);
            rect.dx = ClampDX(rect.dx + offsetY);
            break;
        }

        case ResizeTypeRightBottom:
        {
            rect.dx = ClampDX(rect.dx + offsetY);
            rect.dy = ClampDY(rect.dy - offsetX);
            break;
        }

		default:
        {
            break;
        }
    }

    return rect;
}

Rect UIControlResizeHelper::ResizeControlInQuadrant2(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta)
{
    Rect rect = resizeRect;
    const UIGeometricData &gd = uiControl->GetGeometricData();
 
    float32 offsetX = delta.x;
    float32 offsetY = delta.y;
    if (rect.dx + offsetX < MINIMUM_CONTROL_HEIGHT)
    {
        offsetX = rect.dx > MINIMUM_CONTROL_HEIGHT ? MINIMUM_CONTROL_HEIGHT - rect.dx : MINIMUM_CONTROL_HEIGHT;
    }
    if (rect.dy + offsetY < MINIMUM_CONTROL_WIDTH)
    {
        offsetY = rect.dy > MINIMUM_CONTROL_WIDTH ? MINIMUM_CONTROL_WIDTH - rect.dy : MINIMUM_CONTROL_WIDTH;
    }

    switch (unrotatedResizeType)
	{
        case ResizeTypeLeft:
        {
            rect.x -= offsetX * gd.cosA;
            rect.y -= offsetX * gd.sinA;
            rect.dx = ClampDX(rect.dx + offsetX);
            
            break;
        }
            
        case ResizeTypeRight:
        {
            rect.dx = ClampDX(rect.dx - delta.x);
            break;
        }
            
        case ResizeTypeTop:
        {
            rect.x += (offsetY * gd.sinA);
            rect.y -= (offsetY * gd.cosA);
            rect.dy = ClampDY(rect.dy + offsetY);
            break;
        }
            
        case ResizeTypeBottom:
        {
            rect.dy = ClampDY(rect.dy - offsetY);
            break;
        }
            
        case ResizeTypeLeftTop:
        {
            rect.x = rect.x - (offsetX * gd.cosA) + (offsetY * gd.sinA);
            rect.y = rect.y - (offsetX * gd.sinA) - (offsetY * gd.cosA);
            rect.dx = ClampDX(rect.dx + offsetX);
            rect.dy = ClampDY(rect.dy + offsetY);
            
            break;
        }
            
        case ResizeTypeLeftBottom:
        {
            rect.x -= offsetX * gd.cosA;
            rect.y -= offsetX * gd.sinA;
            rect.dx = ClampDX(rect.dx + offsetX);
            rect.dy = ClampDY(rect.dy - offsetY);
            break;
        }
            
        case ResizeTypeRigthTop:
        {
            rect.x += (offsetY * gd.sinA);
            rect.y -= (offsetY * gd.cosA);
            rect.dx = ClampDX(rect.dx - delta.x);
            rect.dy = ClampDY(rect.dy + delta.y);
            break;
        }
            
        case ResizeTypeRightBottom:
        {
            rect.dx = ClampDX(rect.dx - delta.x);
            rect.dy = ClampDX(rect.dy - delta.y);
            break;
        }
            
		default:
        {
            break;
        }
    }
    
    return rect;
}

Rect UIControlResizeHelper::ResizeControlInQuadrant3(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta)
{
    Rect rect = resizeRect;
    const UIGeometricData &gd = uiControl->GetGeometricData();
    
    float32 offsetX = delta.x;
    float32 offsetY = delta.y;
    
    switch (unrotatedResizeType)
	{
        case ResizeTypeLeft:
        {
            rect.dx = ClampDX(rect.dx + offsetY);
            if (rect.dx == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = MINIMUM_CONTROL_WIDTH - resizeRect.dx;
            }
            
            rect.x -= offsetY * gd.cosA;
            rect.y -= offsetY * gd.sinA;
            
            break;
        }

        case ResizeTypeTop:
		{
            rect.dy = ClampDY(rect.dy - offsetX);
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = resizeRect.dy - MINIMUM_CONTROL_HEIGHT;
            }
            
            rect.x -= (offsetX * gd.sinA);
            rect.y += (offsetX * gd.cosA);
            
            break;
        }

        case ResizeTypeRight:
        {
            rect.dx = ClampDX(rect.dx - offsetY);
            break;
        }
            
        case ResizeTypeBottom:
        {
            rect.dy = ClampDY(rect.dy + offsetX);
            break;
        }

        case ResizeTypeLeftTop:
        {
            rect.dx = ClampDX(rect.dx + offsetY);
            rect.dy = ClampDY(rect.dy - offsetX);
            if (rect.dx == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = MINIMUM_CONTROL_WIDTH - resizeRect.dx;
            }
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = resizeRect.dy - MINIMUM_CONTROL_HEIGHT;
            }
            
            rect.x = rect.x - (offsetY * gd.cosA) - (offsetX * gd.sinA);
            rect.y = rect.y - (offsetY * gd.sinA) + (offsetX * gd.cosA);
            break;
        }
            
        case ResizeTypeLeftBottom:
        {
            rect.dx = ClampDY(rect.dx + offsetY);
            if (rect.dx == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = MINIMUM_CONTROL_WIDTH - resizeRect.dx;
            }
            rect.dy = ClampDY(rect.dy + offsetX);
            
            rect.x -= offsetY * gd.cosA;
            rect.y -= offsetY * gd.sinA;

            break;
        }

        case ResizeTypeRigthTop:
        {
            rect.dy = ClampDY(rect.dy - offsetX);
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = resizeRect.dy - MINIMUM_CONTROL_HEIGHT;
            }
            rect.dx = ClampDX(rect.dx - offsetY);
    
            rect.x -= (offsetX * gd.sinA);
            rect.y += (offsetX * gd.cosA);

            break;
        }
            
        case ResizeTypeRightBottom:
        {
            rect.dx = ClampDX(rect.dx - offsetY);
            rect.dy = ClampDY(rect.dy + offsetX);
            break;
        }

		default:
        {
            break;
        }
    }

    return rect;
}

};