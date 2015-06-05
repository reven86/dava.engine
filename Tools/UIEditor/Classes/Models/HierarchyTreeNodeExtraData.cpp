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


#include "HierarchyTreeNodeExtraData.h"

using namespace DAVA;

HierarchyTreeNodeExtraData::HierarchyTreeNodeExtraData()
{
}

// Access to Localization Key.
bool HierarchyTreeNodeExtraData::IsLocalizationKeyExist(UIControl::eControlState state) const
{
    return localizationKeysMap.find(state) != localizationKeysMap.end();
}

void HierarchyTreeNodeExtraData::SetLocalizationKey(const WideString& localizationKey, UIControl::eControlState state)
{
    this->localizationKeysMap[state] = localizationKey;
}
    
WideString HierarchyTreeNodeExtraData::GetLocalizationKey(UIControl::eControlState state) const
{
    if (!ValidateControlState(state))
    {
        return WideString();
    }
    
    Map<UIControl::eControlState, WideString>::const_iterator iter = this->localizationKeysMap.find(state);
    return iter->second;
}

bool HierarchyTreeNodeExtraData::ValidateControlState(UIControl::eControlState state) const
{
    Map<UIControl::eControlState, WideString>::const_iterator iter = this->localizationKeysMap.find(state);
    if (iter == this->localizationKeysMap.end())
    {
        Logger::Error("UIControlState %i is invalid in HierarchyTreeNodeExtraData!", state);
        return false;
    }
    
    return true;
}

void HierarchyTreeNodeExtraData::SetStatePropertyDirty(UIControl::eControlState controlState,
                                                       const QString& propertyName, bool value)
{
    STATEDIRTYPROPERTIESITER iter = this->stateDirtyProperties.find(controlState);
    if (iter == stateDirtyProperties.end())
    {
        Set<QString> propertiesSet;
        if (value)
        {
            propertiesSet.insert(propertyName);
        }

        this->stateDirtyProperties.insert(std::make_pair(controlState, propertiesSet));
        return;
    }
    
    Set<QString>& propertiesSet = this->stateDirtyProperties[controlState];
    if (value)
    {
        propertiesSet.insert(propertyName);
    }
    else
    {
        propertiesSet.erase(propertyName);
    }
}

bool HierarchyTreeNodeExtraData::IsStatePropertyDirty(UIControl::eControlState controlState,
                                                      const QString& propertyName)
{
    STATEDIRTYPROPERTIESITER iter = this->stateDirtyProperties.find(controlState);
    if (iter == stateDirtyProperties.end())
    {
        return false;
    }
    
    Set<QString>::iterator innerIter = iter->second.find(propertyName);
    return (innerIter != iter->second.end());
}

bool HierarchyTreeNodeExtraData::IsStatePropertyDirtyMapEmpty(UIControl::eControlState controlState)
{
    STATEDIRTYPROPERTIESITER iter = this->stateDirtyProperties.find(controlState);
    if (iter == stateDirtyProperties.end())
    {
        // No changes for this property at all.
        return true;
    }
    
    return (iter->second.size() == 0);
}
