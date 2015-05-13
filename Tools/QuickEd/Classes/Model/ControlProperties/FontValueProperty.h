#ifndef __UI_EDITOR_FONT_VALUE_PROPERTY__
#define __UI_EDITOR_FONT_VALUE_PROPERTY__

#include "ValueProperty.h"

class FontValueProperty : public ValueProperty
{
public:
    FontValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, ValueProperty *sourceProperty, eCopyType copyType);
    virtual ~FontValueProperty();
    
    int GetCount() const override;
    BaseProperty *GetProperty(int index) const override;

    DAVA::VariantType GetValue() const override;
    void RefreshFontValue();
protected:
    void ApplyValue(const DAVA::VariantType &value) override;
    
protected:
    DAVA::String presetName;
};

#endif // __UI_EDITOR_FONT_VALUE_PROPERTY__
