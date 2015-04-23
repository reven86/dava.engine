#ifndef __QUICKED_PROPERTY_LISTENER_H__
#define __QUICKED_PROPERTY_LISTENER_H__

class AbstractProperty;
class RootProperty;
class ComponentPropertiesSection;

class PropertyListener
{
public:
    virtual void PropertyChanged(AbstractProperty *property) = 0;

    virtual void ComponentPropertiesWillBeAdded(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;
    virtual void ComponentPropertiesWasAdded(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;

    virtual void ComponentPropertiesWillBeRemoved(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;
    virtual void ComponentPropertiesWasRemoved(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;
};

#endif // __QUICKED_PROPERTY_LISTENER_H__
