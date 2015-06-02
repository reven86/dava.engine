/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __UI_EDITOR_ABSTRACT_PROPERTY_H__
#define __UI_EDITOR_ABSTRACT_PROPERTY_H__

#include "Base/BaseObject.h"

class PackageSerializer;

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
        EF_CAN_RESET = 0x01,
        EF_ADD_REMOVE = 0x02,
        EF_INHERITED = 0x04,
        EF_CAN_REMOVE = 0x08,
        EF_CAN_CREATE = 0x10,
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
    AbstractProperty *GetParent() const;
    void SetParent(AbstractProperty *parent);
    
    virtual int GetCount() const = 0;
    virtual AbstractProperty *GetProperty(int index) const = 0;
    virtual int GetIndex(AbstractProperty *property) const;

    virtual void Refresh();
    virtual AbstractProperty *FindPropertyByPrototype(AbstractProperty *prototype);
    virtual bool HasChanges() const;
    virtual void Serialize(PackageSerializer *serializer) const = 0;

    virtual const DAVA::String &GetName() const = 0;
    virtual ePropertyType GetType() const = 0;
    virtual DAVA::uint32 GetFlags() const { return 0; };

    virtual bool IsReadOnly() const;

    virtual DAVA::VariantType GetValue() const;
    virtual void SetValue(const DAVA::VariantType &newValue);
    virtual DAVA::VariantType GetDefaultValue() const;
    virtual void SetDefaultValue(const DAVA::VariantType &newValue);
    virtual const EnumMap *GetEnumMap() const;
    virtual void ResetValue();
    virtual bool IsReplaced() const;

    AbstractProperty *GetRootProperty();
    const AbstractProperty *GetRootProperty() const;

private:
    AbstractProperty *parent;
};


#endif // __UI_EDITOR_ABSTRACT_PROPERTY_H__
