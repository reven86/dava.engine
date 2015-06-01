#ifndef __UI_EDITOR_FONT_VALUE_PROPERTY__
#define __UI_EDITOR_FONT_VALUE_PROPERTY__

#include "IntrospectionProperty.h"

class FontValueProperty : public IntrospectionProperty
{
public:
    FontValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, FontValueProperty *sourceProperty, eCloneType copyType);
    virtual ~FontValueProperty();
    
    int GetCount() const override;
    AbstractProperty *GetProperty(int index) const override;

    void Refresh() override;

    DAVA::VariantType GetValue() const override;
protected:
    void ApplyValue(const DAVA::VariantType &value) override;
    
protected:
    DAVA::String presetName;
};

#endif // __UI_EDITOR_FONT_VALUE_PROPERTY__
