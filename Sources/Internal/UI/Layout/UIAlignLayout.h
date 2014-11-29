//
//  UIAlignLayout.h
//  Framework
//
//  Created by Dmitry Belsky on 29.11.14.
//
//

#ifndef __DAVAENGINE_UI_ALIGN_LAYOUT_H__
#define __DAVAENGINE_UI_ALIGN_LAYOUT_H__

#include "UILayout.h"

namespace DAVA
{
    class UIAlignLayout : public UILayout
    {
    public:
        UIAlignLayout();
        virtual ~UIAlignLayout();
        
        virtual void Apply(UIControl *control) override;
    };
}

#endif // __DAVAENGINE_UI_ALIGN_LAYOUT_H__
