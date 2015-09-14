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


#include "UIComponent.h"
#include "UI/UIControl.h"

#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"

namespace DAVA
{

UIComponent::UIComponent()
    : control(nullptr)
{
}

UIComponent::UIComponent(const UIComponent &src)
    : control(nullptr)
{
    
}

UIComponent::~UIComponent()
{
}

UIComponent &UIComponent::operator=(const UIComponent &src)
{
    return *this;
}

UIComponent * UIComponent::CreateByType(uint32 componentType)
{
    switch (componentType)
    {
        case LINEAR_LAYOUT_COMPONENT:
            return new UILinearLayoutComponent();
            
        case IGNORE_LAYOUT_COMPONENT:
            return new UIIgnoreLayoutComponent();
            
        case SIZE_POLICY_COMPONENT:
            return new UISizePolicyComponent();
            
        case ANCHOR_COMPONENT:
            return new UIAnchorComponent();
            
        default:
            DVASSERT(false);
            return nullptr;
    }
    
}
    
bool UIComponent::IsMultiple(uint32 componentType)
{
    switch (componentType)
    {
        case LINEAR_LAYOUT_COMPONENT:
            return false;
            
        case IGNORE_LAYOUT_COMPONENT:
            return false;
            
        case SIZE_POLICY_COMPONENT:
            return false;
            
        case ANCHOR_COMPONENT:
            return false;
            
        default:
            DVASSERT(false);
            return false;
    }
    
}


}
