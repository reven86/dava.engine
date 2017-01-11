#ifndef __QUICKED_VISIBLE_VALUE_PROPERTY__
#define __QUICKED_VISIBLE_VALUE_PROPERTY__

#include "IntrospectionProperty.h"

class VisibleValueProperty : public IntrospectionProperty
{
public:
    VisibleValueProperty(DAVA::BaseObject* object, const DAVA::InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType copyType);
    ~VisibleValueProperty() override = default;

    void SetVisibleInEditor(bool visible);
    bool GetVisibleInEditor() const;

    DAVA::VariantType GetValue() const override;

protected:
    void ApplyValue(const DAVA::VariantType& value) override;

protected:
    bool visibleInEditor = true;
    bool visibleInGame = true;
};

#endif // __QUICKED_VISIBLE_VALUE_PROPERTY__
