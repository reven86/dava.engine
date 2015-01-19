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

#ifndef __DAVAENGINE_RENDER_2D_COMPONENT_H__
#define __DAVAENGINE_RENDER_2D_COMPONENT_H__

#include "UIComponent.h"
#include "Base/Function.h"

namespace DAVA
{
class UIControl;
class UIGeometricData;

class UIRenderComponent : public UIComponent
{
public:
    typedef Function<void(const UIGeometricData&)> RenderFunctor;

    IMPLEMENT_COMPONENT_TYPE(Component::UI_RENDER_COMPONENT);

    UIRenderComponent();
    virtual ~UIRenderComponent();

    virtual Component * Clone(UIControl* toControl);

    inline void SetCustomDraw(const RenderFunctor& f);
    inline void SetCustomDrawAfterChilds(const RenderFunctor& f);
    inline void SetCustomBeforeSystemDraw(const RenderFunctor& f);
    inline void SetCustomAfterSystemDraw(const RenderFunctor& f);
    inline void SetCustomSystemDraw(const RenderFunctor& f);
    inline const RenderFunctor& GetCustomDraw() const;
    inline const RenderFunctor& GetCustomDrawAfterChilds() const;
    inline const RenderFunctor& GetCustomBeforeSystemDraw() const;
    inline const RenderFunctor& GetCustomAfterSystemDraw() const;
    inline const RenderFunctor& GetCustomSystemDraw() const;

private:
    RenderFunctor customDraw;
    RenderFunctor customDrawAfterChilds;
    RenderFunctor customBeforeSystemDraw;
    RenderFunctor customAfterSystemDraw;
    RenderFunctor customSystemDraw;

};

void UIRenderComponent::SetCustomDraw(const RenderFunctor& f)
{
    customDraw = f;
}

void UIRenderComponent::SetCustomDrawAfterChilds(const RenderFunctor& f)
{
    customDrawAfterChilds = f;
}

void UIRenderComponent::SetCustomBeforeSystemDraw(const RenderFunctor& f)
{
    customBeforeSystemDraw = f;
}

void UIRenderComponent::SetCustomAfterSystemDraw(const RenderFunctor& f)
{
    customAfterSystemDraw = f;
}

void UIRenderComponent::SetCustomSystemDraw(const RenderFunctor& f)
{
    customSystemDraw = f;
}

const UIRenderComponent::RenderFunctor& UIRenderComponent::GetCustomDraw() const
{
    return customDraw;
}

const UIRenderComponent::RenderFunctor& UIRenderComponent::GetCustomDrawAfterChilds() const
{
    return customDrawAfterChilds;
}

const UIRenderComponent::RenderFunctor& UIRenderComponent::GetCustomBeforeSystemDraw() const
{
    return customBeforeSystemDraw;
}

const UIRenderComponent::RenderFunctor& UIRenderComponent::GetCustomAfterSystemDraw() const
{
    return customAfterSystemDraw;
}

const UIRenderComponent::RenderFunctor& UIRenderComponent::GetCustomSystemDraw() const
{
    return customSystemDraw;
}

}

#endif //__DAVAENGINE_RENDER_2D_COMPONENT_H__