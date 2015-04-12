#ifndef __QUICKED_PROPERTY_LISTENER_H__
#define __QUICKED_PROPERTY_LISTENER_H__

class BaseProperty;
class PropertiesRoot;
class ComponentPropertiesSection;

class PropertyListener
{
public:
    virtual void PropertyChanged(BaseProperty *property) = 0;

    virtual void ComponentPropertiesWillBeAdded(PropertiesRoot *root, ComponentPropertiesSection *section, int index) = 0;
    virtual void ComponentPropertiesWasAdded(PropertiesRoot *root, ComponentPropertiesSection *section, int index) = 0;

    virtual void ComponentPropertiesWillBeRemoved(PropertiesRoot *root, ComponentPropertiesSection *section, int index) = 0;
    virtual void ComponentPropertiesWasRemoved(PropertiesRoot *root, ComponentPropertiesSection *section, int index) = 0;
};

#endif // __QUICKED_PROPERTY_LISTENER_H__
