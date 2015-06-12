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

    static float32 deg45InRad = DegToRad(45.0); // 45 degrees
    float32 sin45 = 0.0f;
    float32 cos45 = 0.0f;
    SinCosFast(deg45InRad, sin45, cos45);
    
    if ((cosAngle >= 0) && (sinAngle <= sin45) && (sinAngle >= -sin45))
    {
        return QUADRANT_MINUS_PI_4_TO_PI_4;
    }
    else if ((sinAngle > 0) && (fabs(cosAngle) < cos45))
    {
        return QUADRANT_PI_4_TO_3PI_4;
    }
    else if ((cosAngle < 0) && (sinAngle <= sin45) && (sinAngle >= -sin45))
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

    eRotationQuadrant quadrant = GetRotationQuadant(uiControl->GetGeometricData().angle);
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
    float32 sinA = 0.0f;
    float32 cosA = 0.0f;
    SinCosFast(uiControl->angle, sinA, cosA);

    float32 offsetX = delta.x;
    float32 offsetY = delta.y;

    switch (unrotatedResizeType)
	{
        case ResizeTypeLeft:
        {
            rect.dx = ClampDX(rect.dx - offsetX);
            if(rect.dx == MINIMUM_CONTROL_HEIGHT)
            {
                 offsetX = resizeRect.dx - MINIMUM_CONTROL_HEIGHT;
            }
            
            rect.x += offsetX * cosA;
            rect.y += offsetX * sinA;
            break;
        }

        case ResizeTypeRight:
        {
            rect.dx = ClampDX(rect.dx + delta.x);
            break;
        }

        case ResizeTypeTop:
        {
            rect.dy = ClampDY(rect.dy - offsetY);
            if(rect.dy == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = resizeRect.dy - MINIMUM_CONTROL_WIDTH;
            }
            rect.x -= (offsetY * sinA);
            rect.y += (offsetY * cosA);
            break;
        }

        case ResizeTypeBottom:
        {
            rect.dy = ClampDY(rect.dy + offsetY);
            break;
        }

        case ResizeTypeLeftTop:
        {
            rect.dx = ClampDX(rect.dx - offsetX);
            rect.dy = ClampDY(rect.dy - offsetY);
            if(rect.dx == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = resizeRect.dx - MINIMUM_CONTROL_HEIGHT;
            }
            if(rect.dy == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = resizeRect.dy - MINIMUM_CONTROL_WIDTH;
            }
            rect.x = rect.x + (offsetX * cosA) - (offsetY * sinA);
            rect.y = rect.y + (offsetX * sinA) + (offsetY * cosA);
            break;
        }

        case ResizeTypeLeftBottom:
        {
            rect.dx = ClampDX(rect.dx - offsetX);
            if(rect.dx == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = resizeRect.dx - MINIMUM_CONTROL_HEIGHT;
            }
            rect.x += offsetX * cosA;
            rect.y += offsetX * sinA;
            rect.dy = ClampDY(rect.dy + offsetY);
            break;
        }

        case ResizeTypeRigthTop:
        {
            rect.dy = ClampDY(rect.dy - offsetY);
            if(rect.dy == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = resizeRect.dy - MINIMUM_CONTROL_WIDTH;
            }
            rect.x -= (offsetY * sinA);
            rect.y += (offsetY * cosA);
            rect.dx = ClampDX(rect.dx + delta.x);
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

    float32 sinA = 0.0f;
    float32 cosA = 0.0f;
    SinCosFast(uiControl->angle, sinA, cosA);
    
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

            rect.x += offsetY * cosA;
            rect.y += offsetY * sinA;

            break;
        }

        case ResizeTypeTop:
		{
            rect.dy = ClampDY(rect.dy + offsetX);
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = MINIMUM_CONTROL_HEIGHT - resizeRect.dy;
            }

            rect.x += (offsetX * sinA);
            rect.y -= (offsetX * cosA);
            
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

            rect.x = rect.x + (offsetY * cosA) + (offsetX * sinA);
            rect.y = rect.y + (offsetY * sinA) - (offsetX * cosA);
            break;
        }

        case ResizeTypeLeftBottom:
        {
            rect.dx = ClampDY(rect.dx - offsetY);
            if (rect.dx == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = resizeRect.dx - MINIMUM_CONTROL_WIDTH;
            }
            
            rect.x += offsetY * cosA;
            rect.y += offsetY * sinA;
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
            
            rect.x += (offsetX * sinA);
            rect.y -= (offsetX * cosA);
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
    float32 sinA = 0.0f;
    float32 cosA = 0.0f;
    SinCosFast(uiControl->angle, sinA, cosA);
 
    float32 offsetX = delta.x;
    float32 offsetY = delta.y;

    switch (unrotatedResizeType)
	{
        case ResizeTypeLeft:
        {
            rect.dx = ClampDX(rect.dx + offsetX);
            if(rect.dx == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = MINIMUM_CONTROL_HEIGHT - resizeRect.dx;
            }
            rect.x -= offsetX * cosA;
            rect.y -= offsetX * sinA;
            break;
        }
            
        case ResizeTypeRight:
        {
            rect.dx = ClampDX(rect.dx - delta.x);
            break;
        }
            
        case ResizeTypeTop:
        {
            rect.dy = ClampDY(rect.dy + offsetY);
            if(rect.dy == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = MINIMUM_CONTROL_WIDTH - resizeRect.dy;
            }
            rect.x += (offsetY * sinA);
            rect.y -= (offsetY * cosA);
            break;
        }
            
        case ResizeTypeBottom:
        {
            rect.dy = ClampDY(rect.dy - offsetY);
            break;
        }
            
        case ResizeTypeLeftTop:
        {
            rect.dx = ClampDX(rect.dx + offsetX);
            rect.dy = ClampDY(rect.dy + offsetY);
            if(rect.dx == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = MINIMUM_CONTROL_HEIGHT - resizeRect.dx;
            }
            if(rect.dy == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = MINIMUM_CONTROL_WIDTH - resizeRect.dy;
            }
            rect.x = rect.x - (offsetX * cosA) + (offsetY * sinA);
            rect.y = rect.y - (offsetX * sinA) - (offsetY * cosA);
            break;
        }
            
        case ResizeTypeLeftBottom:
        {
            rect.dx = ClampDX(rect.dx + offsetX);
            if(rect.dx == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = MINIMUM_CONTROL_HEIGHT - resizeRect.dx;
            }
            rect.dy = ClampDY(rect.dy - offsetY);
            rect.x -= offsetX * cosA;
            rect.y -= offsetX * sinA;
            break;
        }
            
        case ResizeTypeRigthTop:
        {
            rect.dy = ClampDY(rect.dy + delta.y);
            if(rect.dy == MINIMUM_CONTROL_WIDTH)
            {
                offsetY = MINIMUM_CONTROL_WIDTH - resizeRect.dy;
            }
            rect.dx = ClampDX(rect.dx - delta.x);
            rect.x += (offsetY * sinA);
            rect.y -= (offsetY * cosA);
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
    float32 sinA = 0.0f;
    float32 cosA = 0.0f;
    SinCosFast(uiControl->angle, sinA, cosA);
    
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
            
            rect.x -= offsetY * cosA;
            rect.y -= offsetY * sinA;
            
            break;
        }

        case ResizeTypeTop:
		{
            rect.dy = ClampDY(rect.dy - offsetX);
            if (rect.dy == MINIMUM_CONTROL_HEIGHT)
            {
                offsetX = resizeRect.dy - MINIMUM_CONTROL_HEIGHT;
            }
            
            rect.x -= (offsetX * sinA);
            rect.y += (offsetX * cosA);
            
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
            
            rect.x = rect.x - (offsetY * cosA) - (offsetX * sinA);
            rect.y = rect.y - (offsetY * sinA) + (offsetX * cosA);
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
            
            rect.x -= offsetY * cosA;
            rect.y -= offsetY * sinA;

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
    
            rect.x -= (offsetX * sinA);
            rect.y += (offsetX * cosA);

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