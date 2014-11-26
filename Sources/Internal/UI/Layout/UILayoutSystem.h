//
//  UILayoutSystem.h
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#ifndef __DAVAENGINE_UI_LAYOUT_SYSTEM_H__
#define __DAVAENGINE_UI_LAYOUT_SYSTEM_H__

namespace DAVA
{
    
    class UIControl;
    
    class UILayoutSystem
    {
    public:
        UILayoutSystem();
        virtual ~UILayoutSystem();
        
        void ApplayLayout(UIControl *control);
        
    private:
        void MeasureChildren(UIControl *control);
        void LayoutChildren(UIControl *control);
    };
}

#endif // __DAVAENGINE_UI_LAYOUT_SYSTEM_H__
