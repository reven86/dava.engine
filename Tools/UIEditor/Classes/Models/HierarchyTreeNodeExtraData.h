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


#ifndef __UIEditor__HierarchyTreeNodeExtraData__
#define __UIEditor__HierarchyTreeNodeExtraData__

#include <QString>

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA {
    
// Extra data class for Hierarchy Tree Node.
class HierarchyTreeNodeExtraData
{
    
public:
    HierarchyTreeNodeExtraData();
    
    // Access to Localization Key.
    bool IsLocalizationKeyExist(UIControl::eControlState state) const;
    void SetLocalizationKey(const WideString& localizationKey, UIControl::eControlState state);
    WideString GetLocalizationKey(UIControl::eControlState state) const;

    // Extended Dirty States support - for particular Properties.
    // "State is dirty" means that the content for this state was changed
    // in comparison with default state.
    void SetStatePropertyDirty(UIControl::eControlState controlState, const QString& propertyName, bool value);
    bool IsStatePropertyDirty(UIControl::eControlState controlState, const QString& propertyName);

    // Whether the "state property map" is empty for particular state? This means that
    // the property values for this state are identical to ones in Reference property.
    bool IsStatePropertyDirtyMapEmpty(UIControl::eControlState controlState);
    
private:
    // Validate the state.
    bool ValidateControlState(UIControl::eControlState state) const;

    // Localization Key. This property is state-aware.
    Map<UIControl::eControlState, WideString> localizationKeysMap;
    
    // Extended "dirty" properties list.
    typedef Map<UIControl::eControlState, Set<QString> > STATEDIRTYPROPERTIESMAP;
    typedef STATEDIRTYPROPERTIESMAP::iterator STATEDIRTYPROPERTIESITER;
    typedef STATEDIRTYPROPERTIESMAP::const_iterator STATEDIRTYPROPERTIESCONSTITER;
    
    STATEDIRTYPROPERTIESMAP stateDirtyProperties;
};

};

#endif /* defined(__UIEditor__HierarchyTreeNodeExtraData__) */
