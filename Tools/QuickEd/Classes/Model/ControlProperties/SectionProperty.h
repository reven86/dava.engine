#ifndef __UI_EDITOR_SECTION_PROPERTY_H__
#define __UI_EDITOR_SECTION_PROPERTY_H__

#include "AbstractProperty.h"

class ValueProperty;

class SectionProperty : public AbstractProperty
{
public:
    SectionProperty();

protected:
    virtual ~SectionProperty();
    
public:
    void AddProperty(ValueProperty *section);
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;
    virtual ValueProperty *FindProperty(const DAVA::InspMember *member) const;

    virtual ePropertyType GetType() const {
        return TYPE_HEADER;
    }
    
protected:
    DAVA::Vector<ValueProperty*> children;

};

#endif // __UI_EDITOR_SECTION_PROPERTY_H__
