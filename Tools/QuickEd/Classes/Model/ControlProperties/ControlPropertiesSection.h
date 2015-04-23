#ifndef __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__

#include "SectionProperty.h"

namespace DAVA
{
    class UIControl;
}

class ControlPropertiesSection : public SectionProperty
{
public:
    ControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const ControlPropertiesSection *sourceSection, eCloneType copyType);

protected:
    virtual ~ControlPropertiesSection();

public:
    virtual DAVA::String GetName() const override;

    ValueProperty *GetClassProperty() const { return classProperty; }
    ValueProperty *GetCustomClassProperty() const { return customClassProperty; }
    ValueProperty *GetPrototypeProperty() const { return prototypeProperty; }
    ValueProperty *GetNameProperty() const { return nameProperty; }

private:
    ValueProperty *classProperty;
    ValueProperty *customClassProperty;
    ValueProperty *prototypeProperty;
    ValueProperty *nameProperty;

    DAVA::UIControl *control;
    DAVA::String name;
};

#endif // __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
