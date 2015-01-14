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
#include "UI/UIControl.h"

namespace DAVA
{

class UIRenderComponent : public UIComponent
{
public:
    static const uint32 TYPE = Component::UI_RENDER_COMPONENT;

    UIRenderComponent();
    virtual ~UIRenderComponent();

    virtual Component * Clone(UIControl* toControl);
    virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);

    virtual uint32 GetType() const override;

    inline void SetCustomDraw(Function<void(const UIGeometricData&)>& f);
    inline void SetCustomDrawAfterChilds(Function<void(const UIGeometricData&)>& f);
    inline void SetCustomBeforeSystemDraw(Function<void(const UIGeometricData&)>& f);
    inline void SetCustomAfterSystemDraw(Function<void(const UIGeometricData&)>& f);
    inline void SetCustomSystemDraw(Function<void(const UIGeometricData&)>& f);

    inline const Function<void(const UIGeometricData&)>& GetCustomDraw() const;
    inline const Function<void(const UIGeometricData&)>& GetCustomDrawAfterChilds() const;
    inline const Function<void(const UIGeometricData&)>& GetCustomBeforeSystemDraw() const;
    inline const Function<void(const UIGeometricData&)>& GetCustomAfterSystemDraw() const;
    inline const Function<void(const UIGeometricData&)>& GetCustomSystemDraw() const;

private:
    Function<void(const UIGeometricData&)> customDraw;
    Function<void(const UIGeometricData&)> customDrawAfterChilds;
    Function<void(const UIGeometricData&)> customBeforeSystemDraw;
    Function<void(const UIGeometricData&)> customAfterSystemDraw;
    Function<void(const UIGeometricData&)> customSystemDraw;

};

void UIRenderComponent::SetCustomDraw(Function<void(const UIGeometricData&)>& f)
{
    customDraw = f;
}

void UIRenderComponent::SetCustomDrawAfterChilds(Function<void(const UIGeometricData&)>& f)
{
    customDrawAfterChilds = f;
}

void UIRenderComponent::SetCustomBeforeSystemDraw(Function<void(const UIGeometricData&)>& f)
{
    customBeforeSystemDraw = f;
}

void UIRenderComponent::SetCustomAfterSystemDraw(Function<void(const UIGeometricData&)>& f)
{
    customAfterSystemDraw = f;
}

void UIRenderComponent::SetCustomSystemDraw(Function<void(const UIGeometricData&)>& f)
{
    customSystemDraw = f;
}

const Function<void(const UIGeometricData&)>& UIRenderComponent::GetCustomDraw() const
{
    return customDraw;
}

const Function<void(const UIGeometricData&)>& UIRenderComponent::GetCustomDrawAfterChilds() const
{
    return customDrawAfterChilds;
}

const Function<void(const UIGeometricData&)>& UIRenderComponent::GetCustomBeforeSystemDraw() const
{
    return customBeforeSystemDraw;
}

const Function<void(const UIGeometricData&)>& UIRenderComponent::GetCustomAfterSystemDraw() const
{
    return customAfterSystemDraw;
}

const Function<void(const UIGeometricData&)>& UIRenderComponent::GetCustomSystemDraw() const
{
    return customSystemDraw;
}

}

#endif //__DAVAENGINE_RENDER_2D_COMPONENT_H__