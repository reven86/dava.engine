#ifndef __UI_EDITOR_PROPERTIES_ROOT_H__
#define __UI_EDITOR_PROPERTIES_ROOT_H__

#include "Model/ControlProperties/BaseProperty.h"

class ControlPropertiesSection;
class ComponentPropertiesSection;
class BackgroundPropertiesSection;
class InternalControlPropertiesSection;
class PackageSerializer;
class PropertyListener;

namespace DAVA
{
    class UIControl;
}

class PropertiesRoot : public BaseProperty
{
public:
    PropertiesRoot(DAVA::UIControl *control);
    PropertiesRoot(DAVA::UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType);
    
protected:
    virtual ~PropertiesRoot();
    
public:
    virtual int GetCount() const override;
    virtual BaseProperty *GetProperty(int index) const override;
    
    ControlPropertiesSection *GetControlPropertiesSection(const DAVA::String &name) const;

    bool CanAddComponent(DAVA::uint32 componentType) const;
    bool CanRemoveComponent(DAVA::uint32 componentType) const;
    DAVA::int32 GetIndexOfCompoentPropertiesSection(ComponentPropertiesSection *section) const;
    ComponentPropertiesSection *FindComponentPropertiesSection(DAVA::uint32 componentType) const;
    ComponentPropertiesSection *AddComponentPropertiesSection(DAVA::uint32 componentType);
    void AddComponentPropertiesSection(ComponentPropertiesSection *section);
    void RemoveComponentPropertiesSection(DAVA::uint32 componentType);
    void RemoveComponentPropertiesSection(ComponentPropertiesSection *section);
    
    BackgroundPropertiesSection *GetBackgroundPropertiesSection(int num) const;
    InternalControlPropertiesSection *GetInternalControlPropertiesSection(int num) const;
    
    void AddListener(PropertyListener *listener);
    void RemoveListener(PropertyListener *listener);
    
    void SetProperty(BaseProperty *property, const DAVA::VariantType &newValue);
    void SetDefaultProperty(BaseProperty *property, const DAVA::VariantType &newValue);
    void ResetProperty(BaseProperty *property);
    
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual DAVA::String GetName() const;
    virtual ePropertyType GetType() const;

private:
    void MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const PropertiesRoot *sourceProperties, eCopyType copyType);
    void MakeBackgroundPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType);
    void MakeInternalControlPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties, eCopyType copyType);

private:
    DAVA::UIControl *control;
    DAVA::Vector<ControlPropertiesSection*> controlProperties;
    DAVA::Vector<ComponentPropertiesSection*> componentProperties;
    DAVA::Vector<BackgroundPropertiesSection*> backgroundProperties;
    DAVA::Vector<InternalControlPropertiesSection*> internalControlProperties;
    
    DAVA::Vector<PropertyListener*> listeners;
};

#endif // __UI_EDITOR_PROPERTIES_ROOT_H__
