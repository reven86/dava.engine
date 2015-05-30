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
    ComponentPropertiesSection(DAVA::UIControl *control, DAVA::UIComponent::eType type, DAVA::int32 index, const ComponentPropertiesSection *sourceSection, eCloneType copyType);
protected:
    virtual ~ComponentPropertiesSection();

public:
    DAVA::UIComponent *GetComponent() const;
    DAVA::uint32 GetComponentType() const;
    
    bool HasChanges() const override;
    DAVA::uint32 GetFlags() const override;
    
    void InstallComponent();
    void UninstallComponent();
    
    DAVA::int32 GetComponentIndex() const;
    void RefreshIndex();

    void Accept(PropertyVisitor *visitor) override;

    DAVA::String GetComponentName() const;

private:
    void RefreshName();
    
private:
    DAVA::UIControl *control;
    DAVA::UIComponent *component;
    DAVA::int32 index;
    const ComponentPropertiesSection *prototypeSection;
};

#endif // __QUICKED_COMPONENT_PROPERTIES_SECTION_H__
