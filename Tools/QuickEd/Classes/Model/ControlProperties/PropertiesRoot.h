#ifndef __UI_EDITOR_PROPERTIES_ROOT_H__
#define __UI_EDITOR_PROPERTIES_ROOT_H__

#include "Model/ControlProperties/BaseProperty.h"

class ControlPropertiesSection;
class ComponentPropertiesSection;
class BackgroundPropertiesSection;
class InternalControlPropertiesSection;
class PackageSerializer;

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
    ComponentPropertiesSection *AddComponentPropertiesSection(DAVA::uint32 componentType);
    void RemoveComponentPropertiesSection(ComponentPropertiesSection *section);
    
    BackgroundPropertiesSection *GetBackgroundPropertiesSection(int num) const;
    InternalControlPropertiesSection *GetInternalControlPropertiesSection(int num) const;
    
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
};

#endif // __UI_EDITOR_PROPERTIES_ROOT_H__
