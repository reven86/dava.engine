//
//  UILayout.h
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#ifndef __DAVAENGINE_UI_LAYOUT_H__
#define __DAVAENGINE_UI_LAYOUT_H__

#include "Base/BaseObject.h"
#include "Math/Vector.h"

namespace DAVA
{
    class UIControl;
    
    class UILayout : public BaseObject
    {
    public:
        UILayout();
        virtual ~UILayout();
        
        virtual Vector2 MeasureSize(UIControl *control) = 0;
        virtual void Apply(UIControl *control) = 0;
    };
}

#endif // __DAVAENGINE_UI_LAYOUT_H__
