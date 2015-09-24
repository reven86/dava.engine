/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_UI_COMPONENT_H__
#define __DAVAENGINE_UI_COMPONENT_H__

#include "Base/BaseObject.h"

namespace DAVA
{
class UIControl;

class UIComponent : public BaseObject
{
public:
    enum eType
    {
        LINEAR_LAYOUT_COMPONENT,
        IGNORE_LAYOUT_COMPONENT,
        SIZE_POLICY_COMPONENT,
        ANCHOR_COMPONENT,
        
        COMPONENT_COUNT
    };
    
public:
    UIComponent();
    UIComponent(const UIComponent &src);
    virtual ~UIComponent();
    
    UIComponent &operator=(const UIComponent &src);

    static UIComponent * CreateByType(uint32 componentType);
    static bool IsMultiple(uint32 componentType);

    virtual uint32 GetType() const = 0;

    void SetControl(UIControl* _control);
    UIControl* GetControl() const;

    virtual UIComponent* Clone() const = 0;

private:
    UIControl* control;

public:
    INTROSPECTION_EXTEND(UIComponent, BaseObject, 
        nullptr
    );

};

#define IMPLEMENT_COMPONENT_TYPE(TYPE) \
    virtual uint32 GetType() const { return TYPE; }; \
    static const uint32 C_TYPE = TYPE;

inline void UIComponent::SetControl(UIControl* _control)
{
    control = _control;
}

inline UIControl* UIComponent::GetControl() const
{
    return control;
}
    

}


#endif //__DAVAENGINE_UI_COMPONENT_H__
