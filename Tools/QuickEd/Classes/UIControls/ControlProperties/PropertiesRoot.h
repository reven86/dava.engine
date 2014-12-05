#ifndef __UI_EDITOR_PROPERTIES_ROOT_H__
#define __UI_EDITOR_PROPERTIES_ROOT_H__

#include "UIControls/ControlProperties/BaseProperty.h"

class ControlPropertiesSection;
class BackgroundPropertiesSection;
class InternalControlPropertiesSection;

class PropertiesRoot : public BaseProperty
{
public:
    PropertiesRoot(DAVA::UIControl *control);
    PropertiesRoot(DAVA::UIControl *control, const PropertiesRoot *sourceProperties);
    virtual ~PropertiesRoot();
    
    virtual int GetCount() const override;
    virtual BaseProperty *GetProperty(int index) const override;
    
    ControlPropertiesSection *GetControlPropertiesSection(const DAVA::String &name) const;
    BackgroundPropertiesSection *GetBackgroundPropertiesSection(int num) const;
    InternalControlPropertiesSection *GetInternalControlPropertiesSection(int num) const;
    
    virtual DAVA::String GetName() const;
    virtual ePropertyType GetType() const;

    void AddPropertiesToNode(DAVA::YamlNode *node) const;
    
private:
    void MakeControlPropertiesSection(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const PropertiesRoot *sourceProperties);
    void MakeBackgroundPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties);
    void MakeInternalControlPropertiesSection(DAVA::UIControl *control, const PropertiesRoot *sourceProperties);

private:
    DAVA::Vector<ControlPropertiesSection*> controlProperties;
    DAVA::Vector<BackgroundPropertiesSection*> backgroundProperties;
    DAVA::Vector<InternalControlPropertiesSection*> internalControlProperties;
};

#endif // __UI_EDITOR_PROPERTIES_ROOT_H__
