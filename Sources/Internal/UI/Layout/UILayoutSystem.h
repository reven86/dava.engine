//
//  UILayoutSystem.h
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#ifndef __DAVAENGINE_UI_LAYOUT_SYSTEM_H__
#define __DAVAENGINE_UI_LAYOUT_SYSTEM_H__

#include "Math/Vector.h"
#include "UILayoutConstants.h"

namespace DAVA
{
    
    class UIControl;
    class UILayout;
    
    class UILayoutSystem
    {
    public:
        UILayoutSystem();
        virtual ~UILayoutSystem();
        
        void ApplayLayout(UIControl *control);
        
    private:
        void MeasureChildren(UIControl *control);
        void MeasureControlSize(UIControl *control);
        float GetControlSize(const UIControl *control, eUIOrientation orientation) const;
        float GetControlContentSize(const UIControl *control, eUIOrientation orientation) const;

        void LayoutChildren(UIControl *control);
        
        UILayout *GetLayout(UIControl *control);
    };
}

#endif // __DAVAENGINE_UI_LAYOUT_SYSTEM_H__
