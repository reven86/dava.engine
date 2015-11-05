/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
    virtual DAVA::uint32 GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;

    ClassProperty *GetClassProperty() const { return classProperty; }
    CustomClassProperty *GetCustomClassProperty() const { return customClassProperty; }
    PrototypeNameProperty *GetPrototypeProperty() const { return prototypeProperty; }
    NameProperty *GetNameProperty() const { return nameProperty; }
    
    DAVA::int32 GetControlPropertiesSectionsCount() const;
    ControlPropertiesSection *GetControlPropertiesSection(DAVA::int32 index) const;
    ControlPropertiesSection *GetControlPropertiesSection(const DAVA::String &name) const;

    bool CanAddComponent(DAVA::uint32 componentType) const;
    bool CanRemoveComponent(DAVA::uint32 componentType) const;
    const DAVA::Vector<ComponentPropertiesSection*> &GetComponents() const;
    DAVA::int32 GetIndexOfCompoentPropertiesSection(ComponentPropertiesSection *section) const;
    ComponentPropertiesSection *FindComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 index) const;
    ComponentPropertiesSection *AddComponentPropertiesSection(DAVA::uint32 componentType);
    void AddComponentPropertiesSection(ComponentPropertiesSection *section);
    void RemoveComponentPropertiesSection(DAVA::uint32 componentType, DAVA::uint32 componentIndex);
    void RemoveComponentPropertiesSection(ComponentPropertiesSection *section);

    void AttachPrototypeComponent(ComponentPropertiesSection *section, ComponentPropertiesSection *prototypeSection);
    void DetachPrototypeComponent(ComponentPropertiesSection *section, ComponentPropertiesSection *prototypeSection);

    const DAVA::Vector<BackgroundPropertiesSection*> &GetBackgroundProperties() const;
    BackgroundPropertiesSection *GetBackgroundPropertiesSection(int num) const;

    const DAVA::Vector<InternalControlPropertiesSection*> &GetInternalControlProperties() const;
    InternalControlPropertiesSection *GetInternalControlPropertiesSection(int num) const;
    
    void AddListener(PropertyListener *listener);
    void RemoveListener(PropertyListener *listener);
    
    void SetProperty(AbstractProperty *property, const DAVA::VariantType &newValue);
    void SetDefaultProperty(AbstractProperty *property, const DAVA::VariantType &newValue);
    void ResetProperty(AbstractProperty *property);
    void RefreshProperty(AbstractProperty *property, DAVA::int32 refreshFlags);
    AbstractProperty* FindPropertyByName(const DAVA::String& name) const;

    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor *visitor) override;
    bool IsReadOnly() const override;

    const DAVA::String &GetName() const override;
    ePropertyType GetType() const override;
private:
    void AddBaseProperties(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType);
    void MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const RootProperty *sourceProperties, eCloneType cloneType);
    void MakeBackgroundPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType);
    void MakeInternalControlPropertiesSection(DAVA::UIControl *control, const RootProperty *sourceProperties, eCloneType cloneType);
    DAVA::uint32 GetComponentAbsIndex(DAVA::uint32 componentType, DAVA::uint32 index) const;
    void RefreshComponentIndices();
    
private:
    ControlNode *node; // weak ref

    ClassProperty *classProperty;
    CustomClassProperty *customClassProperty;
    PrototypeNameProperty *prototypeProperty;
    NameProperty *nameProperty;

    DAVA::Vector<ValueProperty *> baseProperties;
    DAVA::Vector<ControlPropertiesSection*> controlProperties;
    DAVA::Vector<ComponentPropertiesSection*> componentProperties;
    DAVA::Vector<BackgroundPropertiesSection*> backgroundProperties;
    DAVA::Vector<InternalControlPropertiesSection*> internalControlProperties;
    
    DAVA::Vector<PropertyListener*> listeners;
};

#endif // __UI_EDITOR_ROOT_PROPERTY_H__
