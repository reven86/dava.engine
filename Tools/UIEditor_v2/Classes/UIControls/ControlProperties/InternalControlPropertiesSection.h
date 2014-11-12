#ifndef __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class InternalControlPropertiesSection : public PropertiesSection
{
public:
    InternalControlPropertiesSection(DAVA::UIControl *control, int num, const InternalControlPropertiesSection *sourceSection);
    virtual ~InternalControlPropertiesSection();

    virtual DAVA::UIControl *GetInternalControl() const;
    void CreateInternalControl();
    
    virtual DAVA::String GetName() const;

    void AddPropertiesToNode(DAVA::YamlNode *node) const;

private:
    DAVA::UIControl *control;
    DAVA::UIControl *internalControl;
    int internalControlNum;
};


#endif // __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
