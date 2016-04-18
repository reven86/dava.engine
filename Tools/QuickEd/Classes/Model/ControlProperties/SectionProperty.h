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


#ifndef __QUICKED_SECTION_PROPERTY_H__
#define __QUICKED_SECTION_PROPERTY_H__

#include "AbstractProperty.h"

template <typename ValueType>
class SectionProperty : public AbstractProperty
{
public:
    SectionProperty(const DAVA::String& sectionName);

protected:
    virtual ~SectionProperty();

public:
    void AddProperty(ValueType* property);
    void InsertProperty(ValueType* property, DAVA::int32 index);
    void RemoveProperty(ValueType* property);

    void AddSection(SectionProperty<ValueType>* section);
    void InsertSection(SectionProperty<ValueType>* section, DAVA::int32 index);
    void RemoveSection(SectionProperty<ValueType>* section);

    DAVA::uint32 GetCount() const override;
    AbstractProperty* GetProperty(DAVA::int32 index) const override;

    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;

    const DAVA::String& GetName() const override;

    virtual ValueType* FindProperty(const DAVA::InspMember* member) const;

    ePropertyType GetType() const override;

protected:
    DAVA::Vector<SectionProperty<ValueType>*> sections;
    DAVA::Vector<ValueType*> children;
    DAVA::String name;
};

template <typename ValueType>
inline SectionProperty<ValueType>::SectionProperty(const DAVA::String& sectionName)
    : name(sectionName)
{
}

template <typename ValueType>
inline SectionProperty<ValueType>::~SectionProperty()
{
    for (ValueType* child : children)
    {
        DVASSERT(child->GetParent() == this);
        child->SetParent(nullptr);
        child->Release();
    }

    for (AbstractProperty* prop : sections)
    {
        DVASSERT(prop->GetParent() == this);
        prop->SetParent(nullptr);
        prop->Release();
    }
}

template <typename ValueType>
inline void SectionProperty<ValueType>::AddProperty(ValueType* property)
{
    DVASSERT(property->GetParent() == nullptr);
    property->SetParent(this);
    children.push_back(SafeRetain(property));
}

template <typename ValueType>
inline void SectionProperty<ValueType>::InsertProperty(ValueType* property, DAVA::int32 index)
{
    DVASSERT(property->GetParent() == nullptr);
    if (0 <= index && index <= static_cast<DAVA::int32>(children.size()))
    {
        property->SetParent(this);
        children.insert(children.begin() + index, SafeRetain(property));
    }
    else
    {
        DVASSERT(false);
    }
}

template <typename ValueType>
inline void SectionProperty<ValueType>::RemoveProperty(ValueType* property)
{
    auto it = std::find(children.begin(), children.end(), property);
    if (it != children.end())
    {
        ValueType* child = *it;
        DVASSERT(child->GetParent() == this);
        child->SetParent(nullptr);
        child->Release();
        children.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

template <typename ValueType>
void SectionProperty<ValueType>::AddSection(SectionProperty<ValueType>* section)
{
    DVASSERT(section->GetParent() == nullptr);
    section->SetParent(this);
    sections.push_back(SafeRetain(section));
}

template <typename ValueType>
void SectionProperty<ValueType>::InsertSection(SectionProperty<ValueType>* section, DAVA::int32 index)
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
void SectionProperty<ValueType>::RemoveSection(SectionProperty<ValueType>* section)
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
inline DAVA::uint32 SectionProperty<ValueType>::GetCount() const
{
    return sections.size() + children.size();
}

template <typename ValueType>
inline AbstractProperty* SectionProperty<ValueType>::GetProperty(DAVA::int32 index) const
{
    if (index < 0 || index >= static_cast<DAVA::int32>(GetCount()))
    {
        DVASSERT(false);
        return nullptr;
    }
    size_t size = sections.size();
    if (index < static_cast<DAVA::int32>(size))
    {
        return sections.at(index);
    }
    return children[index - size];
}

template <typename ValueType>
inline void SectionProperty<ValueType>::Refresh(DAVA::int32 refreshFlags)
{
    for (AbstractProperty* prop : sections)
    {
        prop->Refresh(refreshFlags);
    }
    for (AbstractProperty* prop : children)
    {
        prop->Refresh(refreshFlags);
    }
}

template <typename ValueType>
inline void SectionProperty<ValueType>::Accept(PropertyVisitor* visitor)
{
    // do nothing
}

template <typename ValueType>
const DAVA::String& SectionProperty<ValueType>::GetName() const
{
    return name;
}

template <typename ValueType>
ValueType* SectionProperty<ValueType>::FindProperty(const DAVA::InspMember* member) const
{
    for (ValueType* child : children)
    {
        if (child->IsSameMember(member))
        {
            return child;
        }
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

template <typename ValueType>
AbstractProperty::ePropertyType SectionProperty<ValueType>::GetType() const
{
    return TYPE_HEADER;
}

#endif // __QUICKED_SECTION_PROPERTY_H__
