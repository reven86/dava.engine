#include "SectionProperty.h"

#include "Model/ControlProperties/ValueProperty.h"

using namespace DAVA;

SectionProperty::SectionProperty()
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

ValueProperty *SectionProperty::FindProperty(const DAVA::InspMember *member) const
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        if ((*it)->GetMember() == member)
            return *it;
    }
    return NULL;
}
