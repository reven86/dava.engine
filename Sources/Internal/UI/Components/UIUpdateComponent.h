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

#ifndef __DAVAENGINE_UI_UPDATE_COMPONENT_H__
#define __DAVAENGINE_UI_UPDATE_COMPONENT_H__

#include "UIComponent.h"
#include "Base/Function.h"

namespace DAVA
{
class UIControl;

class UIUpdateComponent : public UIComponent
{
public:
    IMPLEMENT_COMPONENT_TYPE(Component::UI_UPDATE_COMPONENT);

    UIUpdateComponent();
    virtual ~UIUpdateComponent();

    virtual Component* Clone(UIControl * toControl) override;

    inline const Function<void(float32)>& GetCustomUpdate() const;
    inline const Function<bool()>& GetCustomNeedUpdateCheck() const;
    inline const Function<bool()>& GetCustomNeedSystemUpdateCheck() const;
    inline void SetCustomUpdate(const Function<void(float32)>& f);
    inline void SetCustomNeedUpdateCheck(const Function<bool()>& f);
    inline void SetCustomNeedSystemUpdateCheck(const Function<bool()>& f);

private:
    Function<void(float32)> customUpdate;
    Function<bool()> customNeedUpdateCheck;
    Function<bool()> customNeedSystemUpdateCheck;

};

inline const Function<void(float32)>& UIUpdateComponent::GetCustomUpdate() const
{
    return customUpdate;
}

inline const Function<bool()>& UIUpdateComponent::GetCustomNeedUpdateCheck() const
{
    return customNeedUpdateCheck;
}

inline const Function<bool()>& UIUpdateComponent::GetCustomNeedSystemUpdateCheck() const
{
    return customNeedSystemUpdateCheck;
}

inline void UIUpdateComponent::SetCustomUpdate(const Function<void(float32)>& f)
{
    customUpdate = f;
}

inline void UIUpdateComponent::SetCustomNeedUpdateCheck(const Function<bool()>& f)
{
    customNeedUpdateCheck = f;
}

inline void UIUpdateComponent::SetCustomNeedSystemUpdateCheck(const Function<bool()>& f)
{
    customNeedSystemUpdateCheck = f;
}

}

#endif //__DAVAENGINE_UI_UPDATE_COMPONENT_H__