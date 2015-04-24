#ifndef __UI_EDITOR_PROTOTYPE_NAME_PROPERTY__
#define __UI_EDITOR_PROTOTYPE_NAME_PROPERTY__

#include "ValueProperty.h"
#include <Base\Function.h>
#include <UI\UIControl.h>

class PrototypeNameProperty : public ValueProperty
{

public:
    PrototypeNameProperty(const DAVA::String &propName, DAVA::UIControl *anObject, const PrototypeNameProperty *sourceProperty, eCloneType cloneType);

protected:
    virtual ~PrototypeNameProperty();
    
public:
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual ePropertyType GetType() const override;
    virtual DAVA::VariantType GetValue() const override;

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);

protected:
    DAVA::UIControl *object;
};

#endif //__UI_EDITOR_PROTOTYPE_NAME_PROPERTY__
