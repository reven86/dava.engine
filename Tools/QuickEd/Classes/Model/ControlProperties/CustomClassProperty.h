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
    virtual void Refresh() override;
    virtual AbstractProperty *FindPropertyByPrototype(AbstractProperty *prototype) override;
    virtual void Serialize(PackageSerializer *serializer) const override;
    virtual bool IsReadOnly() const override;
    
    virtual ePropertyType GetType() const override;
    virtual DAVA::uint32 GetEditFlag() const  override { return EF_CAN_RESET; };
    virtual DAVA::VariantType GetValue() const override;
    
protected:
    virtual void ApplyValue(const DAVA::VariantType &value) override;
    
protected:
    ControlNode *control; // weak
    const CustomClassProperty *prototypeProperty;
};

#endif // __QUICKED_CUSTOM_CLASS_PROPERTY_H__
