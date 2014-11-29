//
//  UILinearLayout.h
//  Framework
//
//  Created by Dmitry Belsky on 29.11.14.
//
//

#ifndef __DAVAENGINE_UI_LINEAR_LAYOUT_H__
#define __DAVAENGINE_UI_LINEAR_LAYOUT_H__

#include "UILayout.h"
namespace DAVA
{
    class UILinearLayout : public UILayout
    {
    public:
        UILinearLayout();
        virtual ~UILinearLayout();
        
        virtual void Apply(UIControl *control) override;
    };
}

#endif // __DAVAENGINE_UI_LINEAR_LAYOUT_H__
