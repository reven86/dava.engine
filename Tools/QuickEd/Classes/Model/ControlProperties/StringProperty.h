#ifndef __UI_EDITOR_CUSTOM_PROPERTY__
#define __UI_EDITOR_CUSTOM_PROPERTY__

#include "ValueProperty.h"
#include <Base/Function.h>
#include <UI/UIControl.h>

class SubValueProperty;

class StringProperty : public ValueProperty
{
public:
    typedef DAVA::Function<const DAVA::String &(DAVA::UIControl *)> Getter;
    typedef DAVA::Function<void(DAVA::UIControl *, const DAVA::String &)> Setter;

public:
    StringProperty(const DAVA::String &propName, DAVA::UIControl *object, const Getter &getter, const Setter &setter, const StringProperty *sourceProperty, eCloneType cloneType);

protected:
    virtual ~StringProperty();
    
public:
    virtual void Serialize(PackageSerializer *serializer) const override;
    virtual DAVA::uint32 GetEditFlag() const override{ return editFlags; };

    virtual ePropertyType GetType() const override;
    virtual DAVA::VariantType GetValue() const override;

public:
    void SetEditFlag(DAVA::uint32 newEditFlags);
protected:
    virtual void ApplyValue(const DAVA::VariantType &value);

protected:
    DAVA::UIControl *object;
    Getter getter;
    Setter setter;
    DAVA::uint32 editFlags;
};

#endif //__UI_EDITOR_CUSTOM_PROPERTY__
