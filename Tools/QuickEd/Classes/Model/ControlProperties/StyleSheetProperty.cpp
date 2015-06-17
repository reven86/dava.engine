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

#include "StyleSheetProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/StyleSheetNode.h"
#include "UI/UIStyleSheet.h"

using namespace DAVA;

StyleSheetProperty::StyleSheetProperty(StyleSheetNode *aStyleSheet, DAVA::uint32 aPropertyIndex)
    : ValueProperty("prop")
    , styleSheet(aStyleSheet) // weak
    , propertyIndex(aPropertyIndex)
{
    const UIStyleSheetPropertyDescriptor& descr = GetStyleSheetPropertyByIndex(propertyIndex);
    name = String(descr.name.c_str());
}

StyleSheetProperty::~StyleSheetProperty()
{
    styleSheet = nullptr; // weak
}

int StyleSheetProperty::GetCount() const
{
    return 0;
}

AbstractProperty *StyleSheetProperty::GetProperty(int index) const
{
    return nullptr;
}

void StyleSheetProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitStyleSheetProperty(this);
}

bool StyleSheetProperty::IsReadOnly() const
{
    return styleSheet->IsReadOnly();
}

AbstractProperty::ePropertyType StyleSheetProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType StyleSheetProperty::GetValue() const
{
    const UIStyleSheet *ss = styleSheet->GetStyleSheet();
    const auto &pairs = ss->GetPropertyTable()->GetProperties();
    for (const auto &pair : pairs)
    {
        if (pair.first == propertyIndex)
        {
            return pair.second;
        }
    }

    DVASSERT(false);
    return VariantType();
}

const EnumMap *StyleSheetProperty::GetEnumMap() const
{
//    auto type = member->Desc().type;
//    
//    if (type == InspDesc::T_ENUM ||
//        type == InspDesc::T_FLAGS)
//        return member->Desc().enumMap;
//    
    return nullptr;
}

void StyleSheetProperty::ApplyValue(const DAVA::VariantType &value)
{
}
