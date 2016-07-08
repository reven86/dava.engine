#ifndef __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
#define __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__

#include "IntrospectionProperty.h"

class LocalizedTextValueProperty : public IntrospectionProperty
{
public:
    LocalizedTextValueProperty(DAVA::BaseObject* object, const DAVA::InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType cloneType);

protected:
    virtual ~LocalizedTextValueProperty();

public:
    void Refresh(DAVA::int32 refreshFlags) override;

    DAVA::VariantType GetValue() const override;

protected:
    void ApplyValue(const DAVA::VariantType& value) override;

protected:
    DAVA::WideString text;
};

#endif // __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
