//
//  UILayoutHintComponent.h
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#ifndef __DAVAENGINE_UI_LAYOUT_HINT_COMPONENT_H__
#define __DAVAENGINE_UI_LAYOUT_HINT_COMPONENT_H__

#include "UIComponent.h"

#include "Math/Vector.h"

namespace DAVA
{
    class UILayoutHintComponent : public UIComponent
    {
    public:
        static const eType TYPE;
        
    public:
        UILayoutHintComponent();
        virtual ~UILayoutHintComponent();
        
        virtual eType GetType() const override;
        
        const Vector2 &GetMeasuredSize() const;
        void SetMeasuredSize(const Vector2 &size);

    private:
        Vector2 measuredSize;
    };
}

#endif // __DAVAENGINE_UI_LAYOUT_HINT_COMPONENT_H__
