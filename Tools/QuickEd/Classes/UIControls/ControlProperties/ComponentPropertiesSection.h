//
//  ComponentPropertiesSection.h
//  QuickEd
//
//  Created by Dmitry Belsky on 30.11.14.
//
//

#ifndef __QUICKED_COMPONENT_PROPERTIES_SECTION__
#define __QUICKED_COMPONENT_PROPERTIES_SECTION__

#include "PropertiesSection.h"

class ComponentPropertiesSection : public PropertiesSection
{
public:
    ComponentPropertiesSection(DAVA::UIControl *control, const DAVA::String &componentName, const ComponentPropertiesSection *sourceSection);
    virtual ~ComponentPropertiesSection();
    
    DAVA::UIComponent *GetComponent() const;
    
    DAVA::String GetName() const;
    
    void AddPropertiesToNode(DAVA::YamlNode *node) const;
    
private:
    DAVA::UIControl *control;
    DAVA::UIComponent *component;
};

#endif // __QUICKED_COMPONENT_PROPERTIES_SECTION__
