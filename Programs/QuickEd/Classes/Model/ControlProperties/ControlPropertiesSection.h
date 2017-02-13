#ifndef __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__

#include "SectionProperty.h"
#include "IntrospectionProperty.h"

namespace DAVA
{
class UIControl;
}

class ControlPropertiesSection : public SectionProperty<IntrospectionProperty>
{
public:
    ControlPropertiesSection(const DAVA::String& name, DAVA::UIControl* control, const DAVA::Type* type, const DAVA::Vector<DAVA::Reflection::Field>& fields, const ControlPropertiesSection* sourceSection, eCloneType copyType);

protected:
    virtual ~ControlPropertiesSection();

public:
    void Accept(PropertyVisitor* visitor) override;

private:
    DAVA::UIControl* control;
};

#endif // __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
