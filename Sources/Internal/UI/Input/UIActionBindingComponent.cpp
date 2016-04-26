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

#include "UIActionBindingComponent.h"

#include "Utils/Utils.h"

namespace DAVA
{
UIActionBindingComponent::UIActionBindingComponent()
{
}

UIActionBindingComponent::UIActionBindingComponent(const UIActionBindingComponent& src)
{
}

UIActionBindingComponent::~UIActionBindingComponent()
{
}

UIActionBindingComponent* UIActionBindingComponent::Clone() const
{
    return new UIActionBindingComponent(*this);
}

const UIActionMap& UIActionBindingComponent::GetActionMap() const
{
    return actionMap;
}

UIActionMap& UIActionBindingComponent::GetActionMap()
{
    return actionMap;
}

const UIInputMap& UIActionBindingComponent::GetInputMap() const
{
    return inputMap;
}

UIInputMap& UIActionBindingComponent::GetInputMap()
{
    return inputMap;
}

bool UIActionBindingComponent::IsBlockOtherKeyboardShortcuts() const
{
    return blockOtherKeyboardShortcuts;
}

void UIActionBindingComponent::SetBlockOtherKeyboardShortcuts(bool block)
{
    blockOtherKeyboardShortcuts = block;
}

String UIActionBindingComponent::GetActionsAsString() const
{
    StringStream stream;
    bool first = true;
    for (const Action& action : actions)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << "; ";
        }
        stream << action.action.c_str() << ", " << action.shortcut1.ToString();
    }
    return stream.str();
}

void UIActionBindingComponent::SetActionsFromString(const String& value)
{
    actions.clear();
    inputMap.Clear();
    Vector<String> actionsStr;
    Split(value, ";", actionsStr);
    for (const String& actionStr : actionsStr)
    {
        Vector<String> str;
        Split(actionStr, ",", str);
        if (str.size() > 1 && !str[0].empty())
        {
            FastName actionName(Trim(str[0]));
            KeyboardShortcut shortcut(Trim(str[1]));
            actions.push_back(Action(actionName, shortcut));
            if (shortcut.GetKey() != Key::UNKNOWN)
                inputMap.BindAction(shortcut, actionName);
        }
    }
}
}
