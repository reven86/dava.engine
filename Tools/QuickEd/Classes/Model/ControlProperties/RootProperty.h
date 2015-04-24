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
class ControlNode;

namespace DAVA
{
    class UIControl;
}

class RootProperty : public AbstractProperty
{
public:
    RootProperty(ControlNode *node, const RootProperty *sourceProperties, eCloneType cloneType);
protected:
    virtual ~RootProperty();
    
public:
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;

    ValueProperty *GetClassProperty() const { return classProperty; }
    ValueProperty *GetCustomClassProperty() const { return customClassProperty; }
    ValueProperty *GetPrototypeProperty() const { return prototypeProperty; }
    ValueProperty *GetNameProperty() const { return nameProperty; }
    
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
    void RefreshProperty(AbstractProperty *property);

    virtual void Refresh() override;
    virtual void Serialize(PackageSerializer *serializer) const override;
    virtual bool IsReadOnly() const override;

    virtual const DAVA::String &GetName() const;
    virtual ePropertyType GetType() const;

private:
    void AddBaseProperties(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType);
    void MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const RootProperty *sourceProperties, eCloneType cloneType);
    void MakeBackgroundPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType);
    void MakeInternalControlPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType);
private:
    ControlNode *node; // weak ref

    ValueProperty *classProperty;
    ValueProperty *customClassProperty;
    ValueProperty *prototypeProperty;
    ValueProperty *nameProperty;

    DAVA::Vector<ValueProperty *> baseProperties;
    DAVA::Vector<ControlPropertiesSection*> controlProperties;
    DAVA::Vector<ComponentPropertiesSection*> componentProperties;
    DAVA::Vector<BackgroundPropertiesSection*> backgroundProperties;
    DAVA::Vector<InternalControlPropertiesSection*> internalControlProperties;
    
    DAVA::Vector<PropertyListener*> listeners;
};

#endif // __UI_EDITOR_ROOT_PROPERTY_H__
