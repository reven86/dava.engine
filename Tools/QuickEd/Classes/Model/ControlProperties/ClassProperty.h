#ifndef __QUICKED_CLASS_PROPERTY_H__
#define __QUICKED_CLASS_PROPERTY_H__

#include "ValueProperty.h"

class ControlNode;

class ClassProperty : public ValueProperty
{
public:
    ClassProperty(ControlNode *control);
    
protected:
    virtual ~ClassProperty();
    
public:
    virtual void Serialize(PackageSerializer *serializer) const override;
    virtual bool IsReadOnly() const override;
    
    virtual ePropertyType GetType() const override;
    virtual DAVA::VariantType GetValue() const override;
    
protected:
    ControlNode *control; // weak
};

#endif // __QUICKED_CLASS_PROPERTY_H__
