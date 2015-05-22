#ifndef __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
#define __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__

#include "IntrospectionProperty.h"

class LocalizedTextValueProperty : public IntrospectionProperty
{
public:
    LocalizedTextValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, const LocalizedTextValueProperty *sourceProperty, eCloneType cloneType);

protected:
    virtual ~LocalizedTextValueProperty();
    
public:
    int GetCount() const override;
    AbstractProperty *GetProperty(int index) const override;

    void Refresh() override;

    DAVA::VariantType GetValue() const override;
protected:
    void ApplyValue(const DAVA::VariantType &value) override;
    
protected:
    DAVA::WideString text;
};

#endif // __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
