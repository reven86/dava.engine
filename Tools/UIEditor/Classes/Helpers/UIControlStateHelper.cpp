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


#include "UIControlStateHelper.h"
using namespace DAVA;

const UIControlStateHelper::CONTROLSTATESMAP UIControlStateHelper::controlStatesMap[] =
{
    {UIControl::STATE_NORMAL,           "Normal"},
    {UIControl::STATE_PRESSED_INSIDE,   "Pressed Inside"},
    {UIControl::STATE_PRESSED_OUTSIDE,  "Pressed Outside"},
    {UIControl::STATE_DISABLED,         "Disabled"},
    {UIControl::STATE_SELECTED,         "Selected"},
    {UIControl::STATE_HOVER,            "Hover"}
};

// Get the list of UIControlStates supported:
int UIControlStateHelper::GetUIControlStatesCount()
{
    return sizeof(controlStatesMap) / sizeof(*controlStatesMap);
}
    
UIControl::eControlState UIControlStateHelper::GetUIControlState(int stateID)
{
    if (ValidateUIControlStateID(stateID) == false)
    {
        return UIControl::STATE_NORMAL;
    }

    return controlStatesMap[stateID].controlState;
}

// Default UI Control State data.
int UIControlStateHelper::GetDefaultControlStateIndex()
{
    return UIControlStateHelper::DEFAULT_UI_CONTROL_STATE_INDEX;
}

QString UIControlStateHelper::GetDefaultControlStateName()
{
    return controlStatesMap[GetDefaultControlStateIndex()].controlStateName;
}

UIControl::eControlState UIControlStateHelper::GetDefaultControlState()
{
    return controlStatesMap[GetDefaultControlStateIndex()].controlState;
}

QString UIControlStateHelper::GetUIControlStateName(UIControl::eControlState controlState)
{
    int statesCount = GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        if (controlState == controlStatesMap[i].controlState)
        {
            return controlStatesMap[i].controlStateName;
        }
    }
    
    Logger::Error("No Control State Name found for Control State %i!", controlState);
    return QString();
}

UIControl::eControlState UIControlStateHelper::GetUIControlStateValue(const QString& controlStateName)
{
    int statesCount = GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        if (controlStateName == controlStatesMap[i].controlStateName)
        {
            return controlStatesMap[i].controlState;
        }
    }
    
    Logger::Error("No Control State found for Control State Name %s!", controlStateName.toStdString().c_str());
    return UIControl::STATE_NORMAL;
}

bool UIControlStateHelper::ValidateUIControlStateID(int stateID)
{
    if (stateID < 0|| stateID >= GetUIControlStatesCount())
    {
        Logger::Error("UI Control State ID %i is invalid!", stateID);
        return false;
    }
    
    return true;
}