//
//  PropertiesSection.h
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#ifndef __UI_EDITOR_PROPERTIES_SECTION_H__
#define __UI_EDITOR_PROPERTIES_SECTION_H__

#include "BaseProperty.h"

class ValueProperty;

class PropertiesSection : public BaseProperty
{
public:
    PropertiesSection();
    virtual ~PropertiesSection();
    
    void AddProperty(ValueProperty *section);
    virtual int GetCount() const override;
    virtual BaseProperty *GetProperty(int index) const override;
    virtual ValueProperty *FindProperty(const DAVA::InspMember *member) const;

    virtual ePropertyType GetType() const {
        return TYPE_HEADER;
    }
    
protected:
    DAVA::Vector<ValueProperty*> children;

};

#endif // __UI_EDITOR_PROPERTIES_SECTION_H__
