//
//  UILayoutComponent.h
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#ifndef __DAVAENGINE_UI_LAYOUT_COMPONENT_H__
#define __DAVAENGINE_UI_LAYOUT_COMPONENT_H__

#include "../UIComponent.h"
#include "Math/Vector.h"

namespace DAVA
{
    class UIControl;
    class UILayout;
    
    class UILayoutComponent : public UIComponent
    {
    public:
        enum eLayoutType {
            LAYOUT_ALIGN,
            LAYOUT_LINEAR
        };
        
    public:
        static const eType TYPE;
        
    public:
        UILayoutComponent();
        virtual ~UILayoutComponent();
        
        virtual eType GetType() const override;
        
        eLayoutType GetLayout() const;
        void SetLayout(eLayoutType _layoutType);
        
    private:
        eLayoutType layoutType;
    };
}

#endif // __DAVAENGINE_UI_LAYOUT_COMPONENT_H__
