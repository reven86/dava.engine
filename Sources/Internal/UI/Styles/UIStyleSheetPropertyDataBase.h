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


#ifndef __DAVAENGINE_UI_STYLESHEET_PROPERTIES_TABLE_H__
#define __DAVAENGINE_UI_STYLESHEET_PROPERTIES_TABLE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/StaticSingleton.h"
#include "FileSystem/VariantType.h"
#include "UI/Styles/UIStyleSheetStructs.h"

namespace DAVA
{

class UIStyleSheetPropertyDataBase : 
    public StaticSingleton<UIStyleSheetPropertyDataBase >
{
public:
    static const int32 STYLE_SHEET_PROPERTY_COUNT = 57;

    UIStyleSheetPropertyDataBase();

    uint32 GetStyleSheetPropertyIndex(const FastName& name) const;
    bool IsValidStyleSheetProperty(const FastName& name) const;
    const UIStyleSheetPropertyDescriptor& GetStyleSheetPropertyByIndex(uint32 index) const;
    int32 FindStyleSheetPropertyByMember(const InspMember* memberInfo) const;

private:
    UIStyleSheetPropertyGroup controlGroup;
    UIStyleSheetPropertyGroup bgGroup;
    UIStyleSheetPropertyGroup staticTextGroup;
    UIStyleSheetPropertyGroup textFieldGroup;

    UIStyleSheetPropertyGroup linearLayoutGroup;
    UIStyleSheetPropertyGroup flowLayoutGroup;
    UIStyleSheetPropertyGroup flowLayoutHintGroup;
    UIStyleSheetPropertyGroup ignoreLayoutGroup;
    UIStyleSheetPropertyGroup sizePolicyGroup;
    UIStyleSheetPropertyGroup anchorGroup;

    Array<UIStyleSheetPropertyDescriptor, STYLE_SHEET_PROPERTY_COUNT> properties; // have to be after groups declaration

    UnorderedMap<FastName, uint32> propertyNameToIndexMap;
};

typedef Bitset<UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT> UIStyleSheetPropertySet;

};


#endif
