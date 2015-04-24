#include "SectionProperty.h"

#include "Model/ControlProperties/ValueProperty.h"

using namespace DAVA;

SectionProperty::SectionProperty(const DAVA::String &sectionName)
    : name(sectionName)
{
    
}

SectionProperty::~SectionProperty()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        DVASSERT((*it)->GetParent() == this);
        (*it)->SetParent(NULL);
        (*it)->Release();
    }
    children.clear();
}

void SectionProperty::AddProperty(ValueProperty *value)
{
    DVASSERT(value->GetParent() == NULL);
    value->SetParent(this);
    children.push_back(SafeRetain(value));
}

int SectionProperty::GetCount() const
{
    return (int) children.size();
}

AbstractProperty *SectionProperty::GetProperty(int index) const
{
    return children[index];
}

void SectionProperty::Refresh()
{
    for (ValueProperty *prop : children)
        prop->Refresh();
}

void SectionProperty::Serialize(PackageSerializer *serializer) const
{
    for (ValueProperty* prop : children)
    {
        prop->Serialize(serializer);
    }
}

const DAVA::String & SectionProperty::GetName() const
{
    return name;
}

ValueProperty *SectionProperty::FindProperty(const DAVA::InspMember *member) const
{
    for (auto child : children)
    {
        if (child->IsSameMember(member))
            return child;
    }
    return nullptr;
}
