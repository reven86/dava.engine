#pragma once

#include "Model/ControlProperties/SectionProperty.h"
#include "Preferences/PreferencesIntrospectionProperty.h"

class PreferencesSectionProperty : public AbstractProperty
{
public:
    PreferencesSectionProperty(const DAVA::String& sectionName);

protected:
    ~PreferencesSectionProperty() override;

public:
    void ApplyPreference();

    void AddProperty(PreferencesIntrospectionProperty* property);
    void InsertProperty(PreferencesIntrospectionProperty* property, DAVA::int32 index);
    void RemoveProperty(PreferencesIntrospectionProperty* property);

    void AddSection(PreferencesSectionProperty* section);
    void InsertSection(PreferencesSectionProperty* section, DAVA::int32 index);
    void RemoveSection(PreferencesSectionProperty* section);

    DAVA::uint32 GetCount() const override;
    AbstractProperty* GetProperty(DAVA::int32 index) const override;

    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;

    const DAVA::String& GetName() const override;

    const DAVA::Type* GetValueType() const override;
    ePropertyType GetType() const override;

protected:
    DAVA::Vector<PreferencesIntrospectionProperty*> children;
    DAVA::Vector<PreferencesSectionProperty*> sections;
    DAVA::String name;
};

inline PreferencesSectionProperty::PreferencesSectionProperty(const DAVA::String& sectionName)
    : AbstractProperty()
    , name(sectionName)
{
}

inline PreferencesSectionProperty::~PreferencesSectionProperty()
{
    for (PreferencesIntrospectionProperty* child : children)
    {
        DVASSERT(child->GetParent() == this);
        child->SetParent(nullptr);
        child->Release();
    }

    for (PreferencesSectionProperty* prop : sections)
    {
        DVASSERT(prop->GetParent() == this);
        prop->SetParent(nullptr);
        prop->Release();
    }
}

inline void PreferencesSectionProperty::ApplyPreference()
{
    for (PreferencesIntrospectionProperty* child : children)
    {
        child->ApplyPreference();
    }

    for (PreferencesSectionProperty* section : sections)
    {
        section->ApplyPreference();
    }
}

inline void PreferencesSectionProperty::AddProperty(PreferencesIntrospectionProperty* property)
{
    DVASSERT(property->GetParent() == nullptr);
    property->SetParent(this);
    children.push_back(SafeRetain(property));
}

inline void PreferencesSectionProperty::InsertProperty(PreferencesIntrospectionProperty* property, DAVA::int32 index)
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

inline void PreferencesSectionProperty::RemoveProperty(PreferencesIntrospectionProperty* property)
{
    auto it = std::find(children.begin(), children.end(), property);
    if (it != children.end())
    {
        PreferencesIntrospectionProperty* prop = *it;
        DVASSERT(prop->GetParent() == this);
        prop->SetParent(nullptr);
        prop->Release();
        children.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

inline void PreferencesSectionProperty::AddSection(PreferencesSectionProperty* section)
{
    DVASSERT(section->GetParent() == nullptr);
    section->SetParent(this);
    sections.push_back(SafeRetain(section));
}

inline void PreferencesSectionProperty::InsertSection(PreferencesSectionProperty* section, DAVA::int32 index)
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

inline void PreferencesSectionProperty::RemoveSection(PreferencesSectionProperty* section)
{
    auto it = std::find(sections.begin(), sections.end(), section);
    if (it != sections.end())
    {
        PreferencesSectionProperty* subSection = *it;
        DVASSERT(subSection->GetParent() == this);
        subSection->SetParent(nullptr);
        subSection->Release();
        sections.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

inline DAVA::uint32 PreferencesSectionProperty::GetCount() const
{
    return static_cast<DAVA::uint32>(children.size() + sections.size());
}

inline AbstractProperty* PreferencesSectionProperty::GetProperty(DAVA::int32 index) const
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

    return children[index - size];
}

inline void PreferencesSectionProperty::Refresh(DAVA::int32 refreshFlags)
{
    for (PreferencesSectionProperty* prop : sections)
    {
        prop->Refresh(refreshFlags);
    }
    for (PreferencesIntrospectionProperty* prop : children)
    {
        prop->Refresh(refreshFlags);
    }
}

inline const DAVA::String& PreferencesSectionProperty::GetName() const
{
    return name;
}

inline const DAVA::Type* PreferencesSectionProperty::GetValueType() const
{
    return nullptr;
}

inline void PreferencesSectionProperty::Accept(PropertyVisitor* /*visitor*/)
{
    // do nothing
}

inline PreferencesSectionProperty::ePropertyType PreferencesSectionProperty::GetType() const
{
    return TYPE_HEADER;
}
