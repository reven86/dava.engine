#ifndef __QUICKED_COMPONENT_PROPERTIES_SECTION_H__
#define __QUICKED_COMPONENT_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UIControl;
}

class ComponentPropertiesSection : public PropertiesSection
{
public:
    ComponentPropertiesSection(DAVA::UIControl *control, DAVA::UIComponent::eType type, const ComponentPropertiesSection *sourceSection, eCopyType copyType);
    virtual ~ComponentPropertiesSection();
    
    DAVA::UIComponent *GetComponent() const;
    
    DAVA::String GetName() const;
    
    virtual bool HasChanges() const override;
    virtual void Serialize(PackageSerializer *serializer) const override;
    
private:
    DAVA::UIControl *control;
    DAVA::UIComponent *component;
    DAVA::UIComponent::eType type;
};

#endif // __QUICKED_COMPONENT_PROPERTIES_SECTION_H__
