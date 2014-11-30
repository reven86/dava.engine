//
//  UILayoutSizeHintComponent.h
//  Framework
//
//  Created by Dmitry Belsky on 28.11.14.
//
//

#ifndef __DAVAENGINE_UI_LAYOUT_SIZE_HINT_COMPONENT_H__
#define __DAVAENGINE_UI_LAYOUT_SIZE_HINT_COMPONENT_H__

#include "UIComponent.h"
#include "Math/Vector.h"
#include "UILayoutConstants.h"

namespace DAVA
{
    
    class UILayoutSizeHintComponent : public UIComponent
    {
    public:
        static const eType TYPE;
        
        enum eSizePolicy {
            FIXED_SIZE,
            SIZE_FROM_CONTENT,
            SIZE_FROM_MAX_CHILDREN,
            SUM_SIZE_FROM_ALL_CHILDREN,
            FIRST_CHILDREN,
            PERCENT_OF_PARENT
        };
        
    public:
        UILayoutSizeHintComponent();
        virtual ~UILayoutSizeHintComponent();
        
        virtual eType GetType() const override;
        virtual UILayoutSizeHintComponent *Clone() const override;

        eSizePolicy GetHorizontalPolicy() const;
        void SetHorizontalPolicy(eSizePolicy policy);
        
        float GetHorizontalValue() const;
        void SetHorizontalValue(float value);
        
        eSizePolicy GetVerticalPolicy() const;
        void SetVerticalPolicy(eSizePolicy policy);
        
        float GetVerticalValue() const;
        void SetVerticalValue(float value);
        
        const Vector2 &GetMeasuredSize() const;
        void SetMeasuredSize(const Vector2 size);
        
        eSizePolicy GetPolicy(eUIOrientation orientation);
        float GetValue(eUIOrientation orientation);
        
    private:
        eSizePolicy horizontalPolicy;
        float horizontalValue;
        
        eSizePolicy verticalPolicy;
        float verticalValue;
        
        Vector2 measuredSize;
        
    private:
        inline int32 GetHorizontalPolicyInt() const;
        inline void SetHorizontalPolicyInt(int32 val);

        inline int32 GetVerticalPolicyInt() const;
        inline void SetVerticalPolicyInt(int32 val);
        
    public:
        INTROSPECTION_EXTEND(UILayoutSizeHintComponent, UIComponent,
                             PROPERTY("horizontalPolicy", InspDesc("Horizontal Policy", GlobalEnumMap<eSizePolicy>::Instance()), GetHorizontalPolicyInt, SetHorizontalPolicyInt, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("horizontalValue", "Horizontal Value", GetHorizontalValue, SetHorizontalValue, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("verticalPolicy", InspDesc("Vertical Policy", GlobalEnumMap<eSizePolicy>::Instance()), GetVerticalPolicyInt, SetVerticalPolicyInt, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("verticalValue", "Vertical Value", GetVerticalValue, SetVerticalValue, I_SAVE | I_VIEW | I_EDIT)
                             );

    };
    
    int32 UILayoutSizeHintComponent::GetVerticalPolicyInt() const
    {
        return GetVerticalPolicy();
    }
    
    void UILayoutSizeHintComponent::SetVerticalPolicyInt(int32 val)
    {
        SetVerticalPolicy((eSizePolicy) val);
    }
    
    int32 UILayoutSizeHintComponent::GetHorizontalPolicyInt() const
    {
        return GetHorizontalPolicy();
    }
    
    void UILayoutSizeHintComponent::SetHorizontalPolicyInt(int32 val)
    {
        SetHorizontalPolicy((eSizePolicy) val);
    }

}

#endif // __DAVAENGINE_UI_LAYOUT_SIZE_HINT_COMPONENT_H__
