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

#ifndef __DAVAENGINE_UI_FOCUS_COMPONENT_H__
#define __DAVAENGINE_UI_FOCUS_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "UI/Focus/FocusHelpers.h"

namespace DAVA
{
class UIControl;

class UIFocusComponent : public UIBaseComponent<UIComponent::FOCUS_COMPONENT>
{
public:
    enum ePolicy
    {
        FOCUSABLE,
        FOCUSABLE_GROUP,
    };

    UIFocusComponent();
    UIFocusComponent(const UIFocusComponent& src);

protected:
    virtual ~UIFocusComponent();

private:
    UIFocusComponent& operator=(const UIFocusComponent&) = delete;

public:
    UIFocusComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool value);

    bool IsRequestFocus() const;
    void SetRequestFocus(bool value);

    ePolicy GetPolicy() const;
    void SetPolicy(ePolicy policy);

    const String& GetNextFocusLeft() const;
    void SetNextFocusLeft(const String& val);

    const String& GetNextFocusRight() const;
    void SetNextFocusRight(const String& val);

    const String& GetNextFocusUp() const;
    void SetNextFocusUp(const String& val);

    const String& GetNextFocusDown() const;
    void SetNextFocusDown(const String& val);

    const String& GetNextControlPathInDirection(FocusHelpers::Direction dir);

    int32 GetTabOrder() const;
    void SetTabOrder(int32 val);

private:
    int32 GetPolicyAsInt() const;
    void SetPolicyFromInt(int32 policy);

private:
    int32 tabOrder = 0;
    ePolicy policy = FOCUSABLE;
    String nextFocusPath[FocusHelpers::DIRECTION_COUNT];
    bool enabled = true;
    bool requestFocus = false;

public:
    INTROSPECTION_EXTEND(UIFocusComponent, UIComponent,
                         PROPERTY("enabled", "Enabled", IsEnabled, SetEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("policy", InspDesc("Policy", GlobalEnumMap<ePolicy>::Instance(), InspDesc::T_ENUM), GetPolicyAsInt, SetPolicyFromInt, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("requestFocus", "Request Focus", IsRequestFocus, SetRequestFocus, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("left", "Next Focus Left", GetNextFocusLeft, SetNextFocusLeft, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("right", "Next Focus Right", GetNextFocusRight, SetNextFocusRight, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("up", "Next Focus Up", GetNextFocusUp, SetNextFocusUp, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("down", "Next Focus Down", GetNextFocusDown, SetNextFocusDown, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("tab", "Tab Order", GetTabOrder, SetTabOrder, I_SAVE | I_VIEW | I_EDIT));
};
}


#endif //__DAVAENGINE_UI_FOCUS_COMPONENT_H__
