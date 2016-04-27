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


#pragma once

#include "Model/ControlProperties/SectionProperty.h"

template <typename ValueType>
class PreferencesSectionProperty : public SectionProperty<ValueType>
{
public:
    PreferencesSectionProperty(const DAVA::String& sectionName);

protected:
    ~PreferencesSectionProperty() override;

public:
    void AddSection(SectionProperty<ValueType>* section);
    void InsertSection(SectionProperty<ValueType>* section, DAVA::int32 index);
    void RemoveSection(SectionProperty<ValueType>* section);

    DAVA::uint32 GetCount() const override;
    AbstractProperty* GetChild(DAVA::int32 index) const;

    void Refresh(DAVA::int32 refreshFlags) override;

    ValueType* FindProperty(const DAVA::InspMember* member) const override;

protected:
    DAVA::Vector<SectionProperty<ValueType>*> sections;
};

template <typename ValueType>
inline PreferencesSectionProperty<ValueType>::PreferencesSectionProperty(const DAVA::String& sectionName)
    : SectionProperty<ValueType>(sectionName)
{
}

template <typename ValueType>
inline PreferencesSectionProperty<ValueType>::~PreferencesSectionProperty()
{
    for (SectionProperty<ValueType>* prop : sections)
    {
        DVASSERT(prop->GetParent() == this);
        prop->SetParent(nullptr);
        prop->Release();
    }
}

template <typename ValueType>
void PreferencesSectionProperty<ValueType>::AddSection(SectionProperty<ValueType>* section)
{
    DVASSERT(section->GetParent() == nullptr);
    section->SetParent(this);
    sections.push_back(SafeRetain(section));
}

template <typename ValueType>
void PreferencesSectionProperty<ValueType>::InsertSection(SectionProperty<ValueType>* section, DAVA::int32 index)
{
    DVASSERT(section->GetParent() == nullptr);
    if (0 <= index && index <= static_cast<DAVA::int32>(sections.size()))
    {
        section->SetParent(this);
        sections.insert(sections.begin() + index, SafeRetain(section));
    }
    else
    {
        DVASSERT(false);
    }
}

template <typename ValueType>
void PreferencesSectionProperty<ValueType>::RemoveSection(SectionProperty<ValueType>* section)
{
    auto it = std::find(sections.begin(), sections.end(), section);
    if (it != sections.end())
    {
        ValueType* child = *it;
        DVASSERT(child->GetParent() == this);
        child->SetParent(nullptr);
        child->Release();
        sections.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

template <typename ValueType>
inline DAVA::uint32 PreferencesSectionProperty<ValueType>::GetCount() const
{
    return SectionProperty<ValueType>::GetCount() + sections.size();
}

template <typename ValueType>
inline AbstractProperty* PreferencesSectionProperty<ValueType>::GetChild(DAVA::int32 index) const
{
    if (index < 0 || index >= static_cast<DAVA::int32>(GetCount()))
    {
        DVASSERT(false);
        return nullptr;
    }
    DAVA::int32 size = static_cast<DAVA::int32>(sections.size());
    if (index < size)
    {
        return sections.at(index);
    }
    return SectionProperty<ValueType>::GetProperty(index - size);
}

template <typename ValueType>
inline void PreferencesSectionProperty<ValueType>::Refresh(DAVA::int32 refreshFlags)
{
    for (SectionProperty<ValueType>* prop : sections)
    {
        prop->Refresh(refreshFlags);
    }
    SectionProperty<ValueType>::Refresh(refreshFlags);
}

template <typename ValueType>
ValueType* PreferencesSectionProperty<ValueType>::FindProperty(const DAVA::InspMember* member) const
{
    ValueType* child = SectionProperty<ValueType>::FindProperty(member);
    if (child != nullptr)
    {
        return child;
    }

    for (SectionProperty<ValueType>* prop : sections)
    {
        ValueType* result = prop->FindProperty(member);
        if (result != nullptr)
        {
            return result;
        }
    }
    return nullptr;
}
