#ifndef __QUICKED_NAME_PROPERTY_H__
#define __QUICKED_NAME_PROPERTY_H__

#include "ValueProperty.h"

class ControlNode;

class NameProperty : public ValueProperty
{
public:
    NameProperty(ControlNode* controlNode, const NameProperty* sourceProperty, eCloneType cloneType);

protected:
    virtual ~NameProperty();

public:
    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;

    bool IsReadOnly() const override;

    ePropertyType GetType() const override;
    DAVA::Any GetValue() const override;

    bool IsOverriddenLocally() const override;

    ControlNode* GetControlNode() const;

protected:
    void ApplyValue(const DAVA::Any& value) override;

protected:
    ControlNode* controlNode; // weak
};

#endif // __QUICKED_NAME_PROPERTY_H__
