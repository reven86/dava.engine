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


#ifndef __UI_EDITOR_VALUE_PROPERTY__
#define __UI_EDITOR_VALUE_PROPERTY__

#include "AbstractProperty.h"

class SubValueProperty;

class ValueProperty : public AbstractProperty
{
public:
    ValueProperty(const DAVA::String &propName);

protected:
    virtual ~ValueProperty();
    
public:
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;

    virtual void Refresh() override;
    virtual bool HasChanges() const override;

    virtual const DAVA::String &GetName() const override;
    virtual ePropertyType GetType() const override;

    virtual DAVA::VariantType GetValue() const override;
    virtual void SetValue(const DAVA::VariantType &newValue) override;
    virtual DAVA::VariantType GetDefaultValue() const override;
    virtual void SetDefaultValue(const DAVA::VariantType &newValue) override;
    virtual void ResetValue() override;
    virtual bool IsReplaced() const override;
    
    virtual DAVA::VariantType GetSubValue(int index) const;
    virtual void SetSubValue(int index, const DAVA::VariantType &newValue);
    virtual DAVA::VariantType GetDefaultSubValue(int index) const;
    virtual void SetDefaultSubValue(int index, const DAVA::VariantType &newValue);

    virtual const EnumMap *GetEnumMap() const override;
    DAVA_DEPRECATED(virtual bool IsSameMember(const DAVA::InspMember *member) const)
    {
        return false;
    }

protected:
    virtual void ApplyValue(const DAVA::VariantType &value);
    
private:
    DAVA::VariantType ChangeValueComponent(const DAVA::VariantType &value, const DAVA::VariantType &component, DAVA::int32 index) const;
    DAVA::VariantType GetValueComponent(const DAVA::VariantType &value, DAVA::int32 index) const;
    
protected:
    DAVA::String name;
    bool replaced;
    DAVA::VariantType defaultValue;
    DAVA::Vector<SubValueProperty*> children;
};

#endif //__UI_EDITOR_VALUE_PROPERTY__
