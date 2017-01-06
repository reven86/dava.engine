#ifndef __QUICKED_NAME_PROPERTY_H__
#define __QUICKED_NAME_PROPERTY_H__

#include "ValueProperty.h"

class ControlNode;

class NameProperty : public ValueProperty
{
public:
    NameProperty(ControlNode* control, const NameProperty* sourceProperty, eCloneType cloneType);

protected:
    virtual ~NameProperty();

public:
    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;

    bool IsReadOnly() const override;

    ePropertyType GetType() const override;
    DAVA::uint32 GetFlags() const override;
    DAVA::VariantType GetValue() const override;

    bool IsOverriddenLocally() const override;

    ControlNode* GetControlNode() const;

protected:
    void ApplyValue(const DAVA::VariantType& value) override;

protected:
    ControlNode* control; // weak
};

#endif // __QUICKED_NAME_PROPERTY_H__
