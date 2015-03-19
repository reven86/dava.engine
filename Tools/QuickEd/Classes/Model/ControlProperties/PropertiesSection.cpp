#include "PropertiesSection.h"

#include "Model/ControlProperties/ValueProperty.h"

using namespace DAVA;

PropertiesSection::PropertiesSection()
{
    
}

PropertiesSection::~PropertiesSection()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        DVASSERT((*it)->GetParent() == this);
        (*it)->SetParent(NULL);
        (*it)->Release();
    }
    children.clear();
}

void PropertiesSection::AddProperty(ValueProperty *value)
{
    DVASSERT(value->GetParent() == NULL);
    value->SetParent(this);
    children.push_back(SafeRetain(value));
}

int PropertiesSection::GetCount() const
{
    return (int) children.size();
}

BaseProperty *PropertiesSection::GetProperty(int index) const
{
    return children[index];
}

ValueProperty *PropertiesSection::FindProperty(const DAVA::InspMember *member) const
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        if ((*it)->GetMember() == member)
            return *it;
    }
    return NULL;
}
