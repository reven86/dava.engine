#ifndef __UI_EDITOR_ABSTRACT_PROPERTY_H__
#define __UI_EDITOR_ABSTRACT_PROPERTY_H__

#include "Base/BaseObject.h"

class PropertyVisitor;

class AbstractProperty : public DAVA::BaseObject
{
public:
    enum ePropertyType
    {
        TYPE_NONE,
        TYPE_HEADER,
        TYPE_VARIANT,
        TYPE_ENUM,
        TYPE_FLAGS,
    };

    enum eEditFrags
    {
        EF_NONE = 0x00,
        EF_CAN_RESET = 0x01,
        EF_INHERITED = 0x02,
        EF_CAN_REMOVE = 0x04,
        EF_AFFECTS_STYLES = 0x08,
        EF_DEPENDS_ON_LAYOUTS = 0x10,
    };

    enum eRefreshFlags
    {
        REFRESH_DEFAULT_VALUE = 0x01,
        REFRESH_LOCALIZATION = 0x02,
        REFRESH_FONT = 0x04,
        REFRESH_DEPENDED_ON_LAYOUT_PROPERTIES = 0x08
    };

    enum eCloneType
    {
        CT_INHERIT,
        CT_COPY
    };

public:
    AbstractProperty();

protected:
    virtual ~AbstractProperty();

public:
    AbstractProperty* GetParent() const;
    void SetParent(AbstractProperty* parent);

    virtual DAVA::uint32 GetCount() const = 0;
    virtual AbstractProperty* GetProperty(DAVA::int32 index) const = 0;
    virtual DAVA::int32 GetIndex(AbstractProperty* property) const;

    virtual void Refresh(DAVA::int32 refreshFlags);
    virtual AbstractProperty* FindPropertyByPrototype(AbstractProperty* prototype);
    virtual bool HasChanges() const;
    virtual void Accept(PropertyVisitor* visitor) = 0;

    virtual const DAVA::String& GetName() const = 0;
    virtual ePropertyType GetType() const = 0;
    virtual DAVA::uint32 GetFlags() const;
    virtual DAVA::int32 GetStylePropertyIndex() const;

    virtual bool IsReadOnly() const;

    virtual DAVA::VariantType::eVariantType GetValueType() const;
    virtual DAVA::VariantType GetValue() const;
    virtual void SetValue(const DAVA::VariantType& newValue);
    virtual DAVA::VariantType GetDefaultValue() const;
    virtual void SetDefaultValue(const DAVA::VariantType& newValue);
    virtual const EnumMap* GetEnumMap() const;
    virtual void ResetValue();
    virtual bool IsOverriddenLocally() const;
    virtual bool IsOverridden() const;

    AbstractProperty* GetRootProperty();
    const AbstractProperty* GetRootProperty() const;

    AbstractProperty* FindPropertyByName(const DAVA::String& name);

private:
    AbstractProperty* parent = nullptr;

public:
    INTROSPECTION_EXTEND(AbstractProperty, DAVA::BaseObject,
                         nullptr
                         );
};


#endif // __UI_EDITOR_ABSTRACT_PROPERTY_H__
