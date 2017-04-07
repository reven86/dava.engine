#ifndef __UI_EDITOR_ROOT_PROPERTY_H__
#define __UI_EDITOR_ROOT_PROPERTY_H__

#include "Model/ControlProperties/AbstractProperty.h"

class ControlPropertiesSection;
class ComponentPropertiesSection;
class BackgroundPropertiesSection;
class InternalControlPropertiesSection;
class PropertyListener;
class ValueProperty;
class NameProperty;
class PrototypeNameProperty;
class ClassProperty;
class CustomClassProperty;
class VisibleValueProperty;
class ControlNode;

namespace DAVA
{
class UIControl;
}

class RootProperty : public AbstractProperty
{
public:
    RootProperty(ControlNode* node, const RootProperty* sourceProperties, eCloneType cloneType);

protected:
    virtual ~RootProperty();

public:
    virtual DAVA::uint32 GetCount() const override;
    virtual AbstractProperty* GetProperty(int index) const override;

    ClassProperty* GetClassProperty() const
    {
        return classProperty;
    }
    CustomClassProperty* GetCustomClassProperty() const
    {
        return customClassProperty;
    }
    PrototypeNameProperty* GetPrototypeProperty() const
    {
        return prototypeProperty;
    }
    NameProperty* GetNameProperty() const
    {
        return nameProperty;
    }
    VisibleValueProperty* GetVisibleProperty() const
    {
        return visibleProperty;
    }

    DAVA::int32 GetControlPropertiesSectionsCount() const;
    ControlPropertiesSection* GetControlPropertiesSection(DAVA::int32 index) const;
    ControlPropertiesSection* GetControlPropertiesSection(const DAVA::String& name) const;

    bool CanAddComponent(DAVA::uint32 componentType) const;
    bool CanRemoveComponent(DAVA::uint32 componentType) const;
    const DAVA::Vector<ComponentPropertiesSection*>& GetComponents() const;
    DAVA::int32 GetIndexOfCompoentPropertiesSection(ComponentPropertiesSection* section) const;
    ComponentPropertiesSection* FindComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 index) const;
    ComponentPropertiesSection* AddComponentPropertiesSection(DAVA::uint32 componentType);
    void AddComponentPropertiesSection(ComponentPropertiesSection* section);
    void RemoveComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex);
    void RemoveComponentPropertiesSection(ComponentPropertiesSection* section);

    void AttachPrototypeComponent(ComponentPropertiesSection* section, ComponentPropertiesSection* prototypeSection);
    void DetachPrototypeComponent(ComponentPropertiesSection* section, ComponentPropertiesSection* prototypeSection);

    void AddListener(PropertyListener* listener);
    void RemoveListener(PropertyListener* listener);

    void SetProperty(AbstractProperty* property, const DAVA::Any& newValue);
    void SetDefaultProperty(AbstractProperty* property, const DAVA::Any& newValue);
    void ResetProperty(AbstractProperty* property);
    void RefreshProperty(AbstractProperty* property, DAVA::int32 refreshFlags);

    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;
    bool IsReadOnly() const override;

    const DAVA::String& GetName() const override;
    const DAVA::Type* GetValueType() const override;
    ePropertyType GetType() const override;

    ControlNode* GetControlNode() const;

private:
    void AddBaseProperties(DAVA::UIControl* control, const RootProperty* sourceProperties, eCloneType cloneType);
    void MakeControlPropertiesSection(DAVA::UIControl* control, const DAVA::Type* type, const DAVA::Vector<DAVA::Reflection::Field>& fields, const RootProperty* sourceProperties, eCloneType cloneType);
    DAVA::uint32 GetComponentAbsIndex(DAVA::uint32 componentType, DAVA::uint32 index) const;
    void RefreshComponentIndices();

private:
    ControlNode* node = nullptr; // weak ref

    ClassProperty* classProperty = nullptr;
    CustomClassProperty* customClassProperty = nullptr;
    PrototypeNameProperty* prototypeProperty = nullptr;
    NameProperty* nameProperty = nullptr;
    VisibleValueProperty* visibleProperty = nullptr; //weak ptr

    DAVA::Vector<ValueProperty*> baseProperties;
    DAVA::Vector<ControlPropertiesSection*> controlProperties;
    DAVA::Vector<ComponentPropertiesSection*> componentProperties;

    DAVA::Vector<PropertyListener*> listeners;
};

#endif // __UI_EDITOR_ROOT_PROPERTY_H__
