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


#include "Preferences/PreferencesRootProperty.h"
#include "QtTools/EditorPreferences/PreferencesStorage.h"

using namespace DAVA;

PreferencesRootProperty::PreferencesRootProperty()
    : AbstractProperty()
{
    const auto& registeredInsp = PreferencesStorage::GetRegisteredInsp();
    for (const InspInfo* info : registeredInsp)
    {
        Section* sectionProperty = new Section(info->Name().c_str());
        sectionProperty->SetParent(this);
        sections.push_back(sectionProperty);

        for (int i = 0, count = info->MembersCount(); i < count; ++i)
        {
            ScopedPtr<PreferencesIntrospectionProperty> inspProp(new PreferencesIntrospectionProperty(info->Member(i)));
            sectionProperty->AddProperty(inspProp);
        }
    }
}

PreferencesRootProperty::~PreferencesRootProperty()
{
    for (auto section : sections)
    {
        section->Release();
    }
}

uint32 PreferencesRootProperty::GetCount() const
{
    return sections.size();
}

AbstractProperty* PreferencesRootProperty::GetProperty(int index) const
{
    DVASSERT(index >= 0);
    DVASSERT(index < sections.size());
    return sections.at(index);
}

void PreferencesRootProperty::Refresh(DAVA::int32 refreshFlags)
{
    for (int32 i = 0, count = static_cast<int32>(GetCount()); i < count; i++)
    {
        GetProperty(i)->Refresh(refreshFlags);
    }
}

void PreferencesRootProperty::Accept(PropertyVisitor*)
{
}

bool PreferencesRootProperty::IsReadOnly() const
{
    return false;
}

const DAVA::String& PreferencesRootProperty::GetName() const
{
    static String rootName = "PREFERENCES ROOT";
    return rootName;
}

AbstractProperty::ePropertyType PreferencesRootProperty::GetType() const
{
    return TYPE_HEADER;
}
