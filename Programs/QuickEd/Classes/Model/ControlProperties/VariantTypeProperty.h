#ifndef __QUICKED_VALUE_PROPERTY_H__
#define __QUICKED_VALUE_PROPERTY_H__

#include "Model/ControlProperties/ValueProperty.h"

class ValueProperty;

class StyleSheetNode;

namespace DAVA
{
class UIControl;
}

class VariantTypeProperty : public ValueProperty
{
public:
    VariantTypeProperty(const DAVA::String& name, DAVA::Any& variantType);

protected:
    virtual ~VariantTypeProperty();

public:
    void Accept(PropertyVisitor* visitor) override;
    bool IsReadOnly() const override;

    ePropertyType GetType() const override;
    DAVA::Any GetValue() const override;
    void ApplyValue(const DAVA::Any& value) override;

private:
    DAVA::Any& value;
};

#endif // __QUICKED_VALUE_PROPERTY_H__
