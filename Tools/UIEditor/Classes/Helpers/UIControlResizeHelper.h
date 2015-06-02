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


#ifndef __UIEditor__ResizeHelper__
#define __UIEditor__ResizeHelper__

#include "DAVAEngine.h"

namespace DAVA {

// Resize type.
enum ResizeType
{
    ResizeTypeNoResize,
	ResizeTypeLeft,
	ResizeTypeRight,
	ResizeTypeTop,
	ResizeTypeBottom,
	ResizeTypeLeftTop,
	ResizeTypeLeftBottom,
	ResizeTypeRigthTop,
	ResizeTypeRightBottom,
	ResizeTypeMove,
};

// Rotation quadrant depending on the angle.
enum eRotationQuadrant
{
    QUADRANT_MINUS_PI_4_TO_PI_4 = 0,
    QUADRANT_PI_4_TO_3PI_4 = 1,
    QUADRANT_3PI_4_TO_4PI_3 = 2,
    QUADRANT_4PI_3_TO_MINUS_PI_4 = 3
};

class UIControlResizeHelper
{
public:
    // Determine the rotation quadrant according to the control's rotation angle.
    static eRotationQuadrant GetRotationQuadant(float32 angle);
    
    // Determine the rotated resize type depending on angle and unrotated one.
    static ResizeType GetRotatedResizeType(ResizeType unrotatedResizeType, UIControl* uiControl);

    // Perform the actual resize.
    static void ResizeControl(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta);
    
protected:
    // Get the "final" resize rect, taking rotation into account.
    static Rect GetResizeRect(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta);
    
    // Clamp the size.
    static float32 ClampDX(float32 rawDX);
    static float32 ClampDY(float32 rawDY);

    // Rotate the resize type 90deg counter-clockwise.
    static ResizeType RotateResizeTypeCounterClockwise(ResizeType originalResizeType);
    
    // Do the actual resizes in different quadrants.
    static Rect ResizeControlInQuadrant0(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta);
    static Rect ResizeControlInQuadrant1(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta);
    static Rect ResizeControlInQuadrant2(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta);
    static Rect ResizeControlInQuadrant3(ResizeType unrotatedResizeType, UIControl* uiControl, const Rect& resizeRect, const Vector2& delta);
};
    
};
#endif /* defined(__UIEditor__ResizeHelper__) */
