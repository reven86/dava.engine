#ifndef __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__

#include "SectionProperty.h"
#include "IntrospectionProperty.h"

namespace DAVA
{
class UIControl;
}

class InternalControlPropertiesSection : public SectionProperty<IntrospectionProperty>
{
public:
    InternalControlPropertiesSection(DAVA::UIControl* control, int num, const InternalControlPropertiesSection* sourceSection, eCloneType copyType);

protected:
    virtual ~InternalControlPropertiesSection();

public:
    virtual DAVA::UIControl* GetInternalControl() const;
    void CreateInternalControl();

    bool HasChanges() const override;
    void Accept(PropertyVisitor* visitor) override;

private:
    DAVA::UIControl* control;
    DAVA::UIControl* internalControl;
    int internalControlNum;
};


#endif // __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
