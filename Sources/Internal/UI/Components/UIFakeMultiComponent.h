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

#ifndef __DAVAENGINE_UI_FAKE_MULTI_COMPONENT_H__
#define __DAVAENGINE_UI_FAKE_MULTI_COMPONENT_H__

#include "UIComponent.h"

namespace DAVA
{
    class UIControl;
    
    class UIFakeMultiComponent : public UIComponent
    {
    public:
        UIFakeMultiComponent();
        UIFakeMultiComponent(UIFakeMultiComponent *src);
        
    protected:
        virtual ~UIFakeMultiComponent();
        
    public:
        IMPLEMENT_COMPONENT_TYPE(FAKE_MULTI_COMPONENT);
        
        virtual UIFakeMultiComponent* Clone() override;
        
        int32 GetValue() const;
        void SetValue(int32 val);

        const String &GetStrValue() const;
        void SetStrValue(const String &val);
        
    private:
        int32 value;
        String strValue;
        
    public:
        INTROSPECTION_EXTEND(UIFakeMultiComponent, UIComponent,
                             PROPERTY("value", "Value", GetValue, SetValue, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("strValue", "String Value", GetStrValue, SetStrValue, I_SAVE | I_VIEW | I_EDIT)
                             );
        
    };
    
    inline int32 UIFakeMultiComponent::GetValue() const
    {
        return value;
    }
    
    inline void UIFakeMultiComponent::SetValue(int32 val)
    {
        value = val;
    }

    inline const String &UIFakeMultiComponent::GetStrValue() const
    {
        return strValue;
    }
    
    inline void UIFakeMultiComponent::SetStrValue(const String &val)
    {
        strValue = val;
    }

}


#endif //__DAVAENGINE_UI_FAKE_MULTI_COMPONENT_H__
