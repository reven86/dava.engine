#ifndef __QUICKED_PROPERTY_LISTENER_H__
#define __QUICKED_PROPERTY_LISTENER_H__

class BaseProperty;

class PropertyListener
{
public:
    virtual void PropertyChanged(BaseProperty *property) = 0;
};

#endif // __QUICKED_PROPERTY_LISTENER_H__
