#ifndef __QUICKED_COMPONENT_PROPERTIES_SECTION_H__
#define __QUICKED_COMPONENT_PROPERTIES_SECTION_H__

#include "SectionProperty.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UIControl;
}

class ComponentPropertiesSection : public SectionProperty
{
public:
    ComponentPropertiesSection(DAVA::UIControl *control, DAVA::UIComponent::eType type, const ComponentPropertiesSection *sourceSection, eCopyType copyType);
    virtual ~ComponentPropertiesSection();
    
    DAVA::UIComponent *GetComponent() const;
    DAVA::uint32 GetComponentType() const;
    
    DAVA::String GetName() const;
    virtual bool CanRemove() const;
    
    virtual bool HasChanges() const override;
    
    void InstallComponent();
    void UninstallComponent();

    virtual void Serialize(PackageSerializer *serializer) const override;
    
private:
    DAVA::UIControl *control;
    DAVA::UIComponent *component;
};

#endif // __QUICKED_COMPONENT_PROPERTIES_SECTION_H__
