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

#ifndef __DAVAENGINE_UI_SIZE_HINT_COMPONENT_H__
#define __DAVAENGINE_UI_SIZE_HINT_COMPONENT_H__

#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UIControl;
    
    class UISizeHintComponent : public UIComponent
    {
    public:
        enum eSizePolicy
        {
            IGNORE,
            FIXED_SIZE,
            PERCENT_OF_CHILDREN_SUM,
            PERCENT_OF_MAX_CHILD,
            PERCENT_OF_FIRST_CHILD,
            PERCENT_OF_LAST_CHILD,
            PERCENT_OF_CONTENT,
            PERCENT_OF_PARENT
        };
    public:
        IMPLEMENT_COMPONENT_TYPE(SIZE_HINT_COMPONENT);
        
        UISizeHintComponent();
        UISizeHintComponent(const UISizeHintComponent &src);
        
    protected:
        virtual ~UISizeHintComponent();
        
    private:
        UISizeHintComponent &operator=(const UISizeHintComponent &) = delete;
        
    public:
        UISizeHintComponent* Clone() override;
        
        eSizePolicy GetHorizontalPolicy() const;
        void SetHorizontalPolicy(eSizePolicy policy);
        
        float GetHorizontalValue() const;
        void SetHorizontalValue(float32 value);
        
        eSizePolicy GetVerticalPolicy() const;
        void SetVerticalPolicy(eSizePolicy policy);
        
        float GetVerticalValue() const;
        void SetVerticalValue(float32 value);
        
        eSizePolicy GetPolicyByAxis(int32 axis) const;
        float GetValueByAxis(int32 axis) const;
        
    private:
        int32 GetHorizontalPolicyAsInt() const;
        void SetHorizontalPolicyFromInt(int32 policy);
        
        int32 GetVerticalPolicyAsInt() const;
        void SetVerticalPolicyFromInt(int32 policy);
        
    private:
        eSizePolicy horizontalPolicy;
        float32 horizontalValue;
        
        eSizePolicy verticalPolicy;
        float32 verticalValue;
        
    public:
        INTROSPECTION_EXTEND(UISizeHintComponent, UIComponent,
                             PROPERTY("horizontalPolicy", InspDesc("Horizontal Policy", GlobalEnumMap<eSizePolicy>::Instance()), GetHorizontalPolicyAsInt, SetHorizontalPolicyFromInt, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("horizontalValue", "Horizontal Value", GetHorizontalValue, SetHorizontalValue, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("verticalPolicy", InspDesc("Vertical Policy", GlobalEnumMap<eSizePolicy>::Instance()), GetVerticalPolicyAsInt, SetVerticalPolicyFromInt, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("verticalValue", "Vertical Value", GetVerticalValue, SetVerticalValue, I_SAVE | I_VIEW | I_EDIT)
                             );
        
    };
    
}


#endif //__DAVAENGINE_UI_SIZE_HINT_COMPONENT_H__
