//
//  UIComponent.h
//  Framework
//
//  Created by Dmitry Belsky on 25.11.14.
//
//

#ifndef __DAVAENGINE_UI_COMPONENT_H__
#define __DAVAENGINE_UI_COMPONENT_H__

#include "Base/BaseObject.h"

namespace DAVA
{
    class UIComponent : public BaseObject
    {
    public:
        enum eType
        {
            COMPONENT_LAYOUT,
            COMPONENT_LAYOUT_SIZE_HINT,
            COMPONENT_LAYOUT_ALIGN_HINT,
        };
        
    public:
        UIComponent();
        virtual ~UIComponent();
        
        virtual eType GetType() const = 0;
    };
}

#endif // __DAVAENGINE_UI_COMPONENT_H__
