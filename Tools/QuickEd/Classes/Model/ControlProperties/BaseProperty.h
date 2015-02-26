#ifndef __UI_EDITOR_BASE_PROPERTY_H__
#define __UI_EDITOR_BASE_PROPERTY_H__

#include "Base/BaseObject.h"

class PackageSerializer;

class BaseProperty : public DAVA::BaseObject
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
        EF_NONE = 0,
        EF_CAN_RESET = 1 << 0,
        EF_ADD_REMOVE = 1 << 1,
    };
    
    enum eCopyType
    {
        COPY_VALUES,
        COPY_FULL
    };
    
public:
    BaseProperty();
    virtual ~BaseProperty();

    BaseProperty *GetParent() const;
    void SetParent(BaseProperty *parent);
    
    virtual int GetCount() const = 0;
    virtual BaseProperty *GetProperty(int index) const = 0;
    virtual int GetIndex(BaseProperty *property) const;

    virtual bool HasChanges() const;
    virtual void Serialize(PackageSerializer *serializer) const;

    virtual DAVA::String GetName() const = 0;
    virtual ePropertyType GetType() const = 0;
    virtual eEditFrags GetEditFlag() const { return EF_NONE; };

    bool IsReadOnly() const;
    void SetReadOnly();
    virtual bool CanRemove() const {return false; }
    virtual bool CanCreate() const {return false; }

    virtual DAVA::VariantType GetValue() const;
    virtual void SetValue(const DAVA::VariantType &newValue);
    virtual DAVA::VariantType GetDefaultValue() const;
    virtual void SetDefaultValue(const DAVA::VariantType &newValue);
    virtual const EnumMap *GetEnumMap() const;
    virtual void ResetValue();
    virtual bool IsReplaced() const;

    DAVA::Vector<DAVA::String> GetPath() const;
    BaseProperty *GetPropertyByPath(const DAVA::Vector<DAVA::String> &path);
    BaseProperty *GetRootProperty();
    const BaseProperty *GetRootProperty() const;

private:
    BaseProperty *parent;
    bool readOnly;
};


#endif // __UI_EDITOR_BASE_PROPERTY_H__
