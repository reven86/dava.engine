#ifndef __UI_EDITOR_FONT_VALUE_PROPERTY__
#define __UI_EDITOR_FONT_VALUE_PROPERTY__

#include "IntrospectionProperty.h"

class FontValueProperty : public IntrospectionProperty
{
public:
    FontValueProperty(DAVA::BaseObject* object, const DAVA::InspMember* member, const IntrospectionProperty* sourceProperty, eCloneType copyType);
    virtual ~FontValueProperty();

    void Refresh(DAVA::int32 refreshFlags) override;

    DAVA::VariantType GetValue() const override;

protected:
    void ApplyValue(const DAVA::VariantType& value) override;

protected:
    DAVA::String presetName;
};

#endif // __UI_EDITOR_FONT_VALUE_PROPERTY__
