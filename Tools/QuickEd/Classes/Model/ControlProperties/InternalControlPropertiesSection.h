#ifndef __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__

#include "SectionProperty.h"

namespace DAVA
{
    class UIControl;
}

class InternalControlPropertiesSection : public SectionProperty
{
public:
    InternalControlPropertiesSection(DAVA::UIControl *control, int num, const InternalControlPropertiesSection *sourceSection, eCloneType copyType);
protected:
    virtual ~InternalControlPropertiesSection();
public:
    virtual DAVA::UIControl *GetInternalControl() const;
    void CreateInternalControl();
    
    virtual DAVA::String GetName() const;
    virtual bool HasChanges() const override;
    virtual void Serialize(PackageSerializer *serializer) const override;

private:
    DAVA::UIControl *control;
    DAVA::UIControl *internalControl;
    int internalControlNum;
};


#endif // __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
