#ifndef __QUICKED_NAME_PROPERTY_H__
#define __QUICKED_NAME_PROPERTY_H__

#include "ValueProperty.h"
#include <Base/Function.h>
#include <UI/UIControl.h>

class ControlNode;

class NameProperty : public ValueProperty
{
public:
    NameProperty(ControlNode *control, const NameProperty *sourceProperty, eCloneType cloneType);
    
protected:
    virtual ~NameProperty();
    
public:
    virtual void Refresh();
    virtual AbstractProperty *FindPropertyByPrototype(AbstractProperty *prototype) override;
    virtual void Serialize(PackageSerializer *serializer) const override;
    virtual bool IsReadOnly() const override;
    
    virtual ePropertyType GetType() const override;
    virtual DAVA::VariantType GetValue() const override;

    virtual bool IsReplaced() const override;

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);
    
protected:
    ControlNode *control; // weak
    const NameProperty *prototypeProperty;
};

#endif // __QUICKED_NAME_PROPERTY_H__
