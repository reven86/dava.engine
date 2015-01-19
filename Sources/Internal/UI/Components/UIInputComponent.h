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

#ifndef __DAVEENGINE_UI_INPUT_COMPONENT_H__
#define __DAVEENGINE_UI_INPUT_COMPONENT_H__

#include "UIComponent.h"
#include "Base/Function.h"

namespace DAVA
{
class UIControl;
class UIEvent;

class UIInputComponent : public UIComponent
{
public:
    IMPLEMENT_COMPONENT_TYPE(Component::UI_INPUT_COMPONENT);

    UIInputComponent();
    virtual ~UIInputComponent();

    virtual Component* Clone(UIControl * toControl) override;

    inline const Function<void(UIEvent*)>& GetCustomInput() const;
    inline void SetCustomInput(const Function<void(UIEvent*)>& val);
    inline const Function<void(UIEvent*)>& GetCustomInputCancelled() const;
    inline void SetCustomInputCancelled(const Function<void(UIEvent*)>& val);
    inline const Function<bool(UIEvent*)>& GetCustomSystemProcessInput() const;
    inline void SetCustomSystemProcessInput(const Function<bool(UIEvent*)>& val);
    inline const Function<void(UIEvent*)>& GetCustomSystemInputCancelled() const;
    inline void SetCustomSystemInputCancelled(const Function<void(UIEvent*)>& val);
    inline const Function<bool(UIEvent*)>& GetCustomSystemInput() const;
    inline void SetCustomSystemInput(const Function<bool(UIEvent*)>& val);

private:
    Function<void(UIEvent*)> customInput;
    Function<void(UIEvent*)> customInputCancelled;
    Function<bool(UIEvent*)> customSystemInput;
    Function<void(UIEvent*)> customSystemInputCancelled;
    Function<bool(UIEvent*)> customSystemProcessInput;

};

inline const Function<void(UIEvent*)>& UIInputComponent::GetCustomInput() const
{
    return customInput;
}

inline void UIInputComponent::SetCustomInput(const Function<void(UIEvent*)>& val)
{
    customInput = val;
}

inline const Function<void(UIEvent*)>& UIInputComponent::GetCustomInputCancelled() const
{
    return customInputCancelled;
}

inline void UIInputComponent::SetCustomInputCancelled(const Function<void(UIEvent*)>& val)
{
    customInputCancelled = val;
}

inline const Function<bool(UIEvent*)>& UIInputComponent::GetCustomSystemProcessInput() const
{
    return customSystemProcessInput;
}

inline void UIInputComponent::SetCustomSystemProcessInput(const Function<bool(UIEvent*)>& val)
{
    customSystemProcessInput = val;
}

inline const Function<void(UIEvent*)>& UIInputComponent::GetCustomSystemInputCancelled() const
{
    return customSystemInputCancelled;
}

inline void UIInputComponent::SetCustomSystemInputCancelled(const Function<void(UIEvent*)>& val)
{
    customSystemInputCancelled = val;
}

inline const Function<bool(UIEvent*)>& UIInputComponent::GetCustomSystemInput() const
{
    return customSystemInput;
}

inline void UIInputComponent::SetCustomSystemInput(const Function<bool(UIEvent*)>& val)
{
    customSystemInput = val;
}

}

#endif //__DAVEENGINE_UI_INPUT_COMPONENT_H__