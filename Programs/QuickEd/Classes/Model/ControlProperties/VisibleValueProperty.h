#ifndef __QUICKED_VISIBLE_VALUE_PROPERTY__
#define __QUICKED_VISIBLE_VALUE_PROPERTY__

#include "IntrospectionProperty.h"

class VisibleValueProperty : public IntrospectionProperty
{
public:
    VisibleValueProperty(DAVA::BaseObject* object, const DAVA::String &name, const DAVA::Reflection &ref, const IntrospectionProperty* sourceProperty, eCloneType copyType);
    ~VisibleValueProperty() override = default;

    void SetVisibleInEditor(bool visible);
    bool GetVisibleInEditor() const;

    DAVA::Any GetValue() const override;

protected:
    void ApplyValue(const DAVA::Any& value) override;

protected:
    bool visibleInEditor = true;
    bool visibleInGame = true;
};

#endif // __QUICKED_VISIBLE_VALUE_PROPERTY__
