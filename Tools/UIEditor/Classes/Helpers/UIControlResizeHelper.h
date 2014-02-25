//
//  ResizeHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 12/3/13.
//
//

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
