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
    void Accept(PropertyVisitor *visitor) override;

private:
    DAVA::UIControl *control;
};

#endif // __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
