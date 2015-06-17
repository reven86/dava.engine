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


#include "StyleSheetPropertiesSection.h"

#include "StyleSheetProperty.h"
#include "PropertyVisitor.h"
#include "../PackageHierarchy/StyleSheetNode.h"
#include "UI/UIStyleSheet.h"

using namespace DAVA;

StyleSheetPropertiesSection::StyleSheetPropertiesSection(StyleSheetNode *aStyleSheet)
    : styleSheet(aStyleSheet) // weak
{
    UIStyleSheet *ss = styleSheet->GetStyleSheet();
    const UIStyleSheetPropertyTable *table = ss->GetPropertyTable();
    const Vector<std::pair< uint32, VariantType >> &tableProperties = table->GetProperties();
    for (auto &pair : tableProperties)
    {
        StyleSheetProperty *prop = new StyleSheetProperty(styleSheet, pair.first);
        prop->SetParent(this);
        properties.push_back(prop);
    }
}

StyleSheetPropertiesSection::~StyleSheetPropertiesSection()
{
    styleSheet = nullptr; //weak
    for (StyleSheetProperty *prop : properties)
        SafeRelease(prop);
    properties.clear();
}

int StyleSheetPropertiesSection::GetCount() const
{
    return static_cast<int>(properties.size());
}

AbstractProperty *StyleSheetPropertiesSection::GetProperty(int index) const
{
    return properties[index];
}

void StyleSheetPropertiesSection::Accept(PropertyVisitor *visitor)
{
    visitor->VisitStyleSheetPropertiesSection(this);
}

bool StyleSheetPropertiesSection::IsReadOnly() const
{
    return true;
}

const DAVA::String &StyleSheetPropertiesSection::GetName() const
{
    static String name = "Properties";
    return name;
}

AbstractProperty::ePropertyType StyleSheetPropertiesSection::GetType() const
{
    return TYPE_HEADER;
}
