#ifndef __QUICKED_CUSTOM_CLASS_PROPERTY_H__
#define __QUICKED_CUSTOM_CLASS_PROPERTY_H__

#include "ValueProperty.h"

class ControlNode;

class CustomClassProperty : public ValueProperty
{
public:
    CustomClassProperty(ControlNode *control, const CustomClassProperty *sourceProperty, eCloneType cloneType);
    
protected:
    virtual ~CustomClassProperty();
    
public:
    void Refresh() override;
    AbstractProperty *FindPropertyByPrototype(AbstractProperty *prototype) override;
    void Accept(PropertyVisitor *visitor) override;
    
    bool IsReadOnly() const override;
    
    ePropertyType GetType() const override;
    DAVA::uint32 GetFlags() const  override { return EF_CAN_RESET; };
    DAVA::VariantType GetValue() const override;
    
    const DAVA::String &GetCustomClassName() const;
    bool IsSet() const;
    
protected:
    virtual void ApplyValue(const DAVA::VariantType &value) override;
    
protected:
    ControlNode *control; // weak
    DAVA::String customClass;
    const CustomClassProperty *prototypeProperty;
};

#endif // __QUICKED_CUSTOM_CLASS_PROPERTY_H__
