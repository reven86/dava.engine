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
    
    class UILayoutSizeHintComponent : UIComponent
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
        
        virtual eType GetType() const = 0;
        
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
    };
    
}

#endif // __DAVAENGINE_UI_LAYOUT_SIZE_HINT_COMPONENT_H__
