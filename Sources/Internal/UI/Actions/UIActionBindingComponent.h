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

#ifndef __DAVAENGINE_UI_ACTION_BINDING_COMPONENT_H__
#define __DAVAENGINE_UI_ACTION_BINDING_COMPONENT_H__

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "Input/KeyboardShortcut.h"
#include "Functional/Signal.h"
#include "UIActionMap.h"
#include "UIInputMap.h"

namespace DAVA
{
class UIControl;

class UIActionBindingComponent : public UIBaseComponent<UIComponent::ACTION_BINDING_COMPONENT>
{
public:
    UIActionBindingComponent();
    UIActionBindingComponent(const UIActionBindingComponent& src);

protected:
    virtual ~UIActionBindingComponent();

private:
    UIActionBindingComponent& operator=(const UIActionBindingComponent&) = delete;

public:
    UIActionBindingComponent* Clone() const override;

private:
    String GetActionsAsString() const;
    void SetActionsFromString(const String& value);

    struct Action
    {
        FastName action;
        KeyboardShortcut shortcut1;

        Action(const FastName& action_, const KeyboardShortcut& shortcut_)
            : action(action_)
            , shortcut1(shortcut_)
        {
        }
    };

    Vector<Action> actions;

    UIActionMap actionMap;

public:
    INTROSPECTION_EXTEND(UIActionBindingComponent, UIComponent,
                         PROPERTY("actions", "Actions", GetActionsAsString, SetActionsFromString, I_SAVE | I_VIEW | I_EDIT));
};
}


#endif //__DAVAENGINE_UI_ACTION_BINDING_COMPONENT_H__
