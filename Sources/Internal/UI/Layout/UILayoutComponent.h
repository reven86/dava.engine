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
            LAYOUT_HORIZONTAL,
            LAYOUT_VERTICAL,
        };
        
    public:
        static const eType TYPE;
        
    public:
        UILayoutComponent();
        virtual ~UILayoutComponent();
        
        virtual eType GetType() const override;
        virtual UILayoutComponent *Clone() const override;

        eLayoutType GetLayout() const;
        void SetLayout(eLayoutType _layoutType);
        
    private:
        eLayoutType layoutType;

    private:
        inline int32 GetLayoutAsInt() const;
        inline void SetLayoutAsInt(int32 val);
        
    public:
        INTROSPECTION_EXTEND(UILayoutComponent, UIComponent,
                             PROPERTY("layout", InspDesc("Layout", GlobalEnumMap<eLayoutType>::Instance()), GetLayoutAsInt, SetLayoutAsInt, I_SAVE | I_VIEW | I_EDIT)
                             );

    };
    
    int32 UILayoutComponent::GetLayoutAsInt() const
    {
        return GetLayout();
    }
    
    void UILayoutComponent::SetLayoutAsInt(int32 val)
    {
        SetLayout((eLayoutType) val);
    }

}

#endif // __DAVAENGINE_UI_LAYOUT_COMPONENT_H__
