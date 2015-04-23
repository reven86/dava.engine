#ifndef __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
#define __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__

#include "ValueProperty.h"

class LocalizedTextValueProperty : public ValueProperty
{
public:
    LocalizedTextValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, ValueProperty *sourceProperty, eCopyType copyType);
    virtual ~LocalizedTextValueProperty();
    
    int GetCount() const override;
    AbstractProperty *GetProperty(int index) const override;

    DAVA::VariantType GetValue() const override;
protected:
    void ApplyValue(const DAVA::VariantType &value) override;
    
protected:
    DAVA::WideString text;
};

#endif // __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
