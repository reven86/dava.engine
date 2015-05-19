#ifndef __UI_EDITOR_PROPERTIES_SECTION_H__
#define __UI_EDITOR_PROPERTIES_SECTION_H__

#include "AbstractProperty.h"

class ValueProperty;

class SectionProperty : public AbstractProperty
{
public:
    SectionProperty(const DAVA::String &sectionName);
protected:
    virtual ~SectionProperty();
    
public:
    void AddProperty(ValueProperty *section);
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;
    
    virtual void Refresh() override;
    virtual void Serialize(PackageSerializer *serializer) const;
    virtual const DAVA::String &GetName() const;

    DAVA_DEPRECATED(virtual ValueProperty *FindProperty(const DAVA::InspMember *member) const);

    virtual ePropertyType GetType() const
    {
        return TYPE_HEADER;
    }

protected:
    DAVA::Vector<ValueProperty*> children;
    DAVA::String name;

};

#endif // __UI_EDITOR_PROPERTIES_SECTION_H__
