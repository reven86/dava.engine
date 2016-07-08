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
    VariantTypeProperty(const DAVA::String& name, const DAVA::InspDesc* desc, DAVA::VariantType& variantType);

protected:
    virtual ~VariantTypeProperty();

public:
    void Accept(PropertyVisitor* visitor) override;
    bool IsReadOnly() const override;

    DAVA::VariantType GetValue() const override;
    void ApplyValue(const DAVA::VariantType& value) override;

private:
    DAVA::VariantType& value;
};

#endif // __QUICKED_VALUE_PROPERTY_H__
