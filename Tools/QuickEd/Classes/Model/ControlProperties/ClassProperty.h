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
    void Serialize(PackageSerializer *serializer) const override;
    void Accept(PropertyVisitor *visitor) override;
    
    bool IsReadOnly() const override;
    
    ePropertyType GetType() const override;
    DAVA::VariantType GetValue() const override;
    const DAVA::String &GetClassName() const;
    
protected:
    ControlNode *control; // weak
};

#endif // __QUICKED_CLASS_PROPERTY_H__
