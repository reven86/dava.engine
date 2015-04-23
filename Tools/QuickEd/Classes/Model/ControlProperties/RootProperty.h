#ifndef __UI_EDITOR_ROOT_PROPERTY_H__
#define __UI_EDITOR_ROOT_PROPERTY_H__

#include "Model/ControlProperties/AbstractProperty.h"

class ControlPropertiesSection;
class ComponentPropertiesSection;
class BackgroundPropertiesSection;
class InternalControlPropertiesSection;
class PackageSerializer;
class PropertyListener;
class ValueProperty;

namespace DAVA
{
    class UIControl;
}

class RootProperty : public AbstractProperty
{
public:
    RootProperty(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType);
protected:
    virtual ~RootProperty();
    
public:
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;
    
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
    
    void SetProperty(AbstractProperty *property, const DAVA::VariantType &newValue);
    void SetDefaultProperty(AbstractProperty *property, const DAVA::VariantType &newValue);
    void ResetProperty(AbstractProperty *property);
    
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual DAVA::String GetName() const;
    virtual ePropertyType GetType() const;

private:
    void MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const RootProperty *sourceProperties, eCloneType copyType);
    void MakeBackgroundPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType copyType);
    void MakeInternalControlPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType copyType);
private:
    DAVA::UIControl *control;

    DAVA::Vector<ControlPropertiesSection*> controlProperties;
    DAVA::Vector<ComponentPropertiesSection*> componentProperties;
    DAVA::Vector<BackgroundPropertiesSection*> backgroundProperties;
    DAVA::Vector<InternalControlPropertiesSection*> internalControlProperties;
    
    DAVA::Vector<PropertyListener*> listeners;
};

#endif // __UI_EDITOR_ROOT_PROPERTY_H__
