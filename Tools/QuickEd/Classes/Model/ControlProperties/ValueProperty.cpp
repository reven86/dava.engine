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


#include "ValueProperty.h"

#include "SubValueProperty.h"
#include <Base/BaseMath.h>

using namespace DAVA;


ValueProperty::ValueProperty(const DAVA::String &propName)
    : name(propName)
    , stylePropertyIndex(-1)
    , overridden(false)
    , prototypeProperty(nullptr) // weak
{
}

ValueProperty::~ValueProperty()
{
    for (auto child : children)
        child->Release();
    children.clear();
    
    prototypeProperty = nullptr; // weak

}

uint32 ValueProperty::GetCount() const
{
    return static_cast<int32>(children.size());
}

AbstractProperty* ValueProperty::GetProperty(int32 index) const
{
    if (0 <= index && index < static_cast<int32>(children.size()))
    {
        return children[index];
    }
    else
    {
        DVASSERT(false);
        return nullptr;
    }
}

void ValueProperty::Refresh(DAVA::int32 refreshFlags)
{
    if ((refreshFlags & REFRESH_DEFAULT_VALUE) != 0 && prototypeProperty)
        SetDefaultValue(prototypeProperty->GetValue());

    for (SubValueProperty *prop : children)
        prop->Refresh(refreshFlags);
}

void ValueProperty::AttachPrototypeProperty(const ValueProperty *property)
{
    if (prototypeProperty == nullptr)
    {
        prototypeProperty = property;
    }
    else
    {
        DVASSERT(false);
    }
}

void ValueProperty::DetachPrototypeProperty(const ValueProperty *property)
{
    if (prototypeProperty == property)
    {
        prototypeProperty = nullptr;
    }
    else
    {
        DVASSERT(false);
    }
}

const ValueProperty *ValueProperty::GetPrototypeProperty() const
{
    return prototypeProperty;
}

AbstractProperty *ValueProperty::FindPropertyByPrototype(AbstractProperty *prototype)
{
    return prototype == prototypeProperty ? this : nullptr;
}

bool ValueProperty::HasChanges() const
{
    return IsOverriddenLocally();
}

const DAVA::String &ValueProperty::GetName() const
{
    return name;
}

ValueProperty::ePropertyType ValueProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType ValueProperty::GetValue() const
{
    return VariantType();
}

void ValueProperty::SetValue(const DAVA::VariantType &newValue)
{
    overridden = true;
    ApplyValue(newValue);
}

VariantType ValueProperty::GetDefaultValue() const
{
    return defaultValue;
}

void ValueProperty::SetDefaultValue(const DAVA::VariantType &newValue)
{
    defaultValue = newValue;
    if (!overridden)
        ApplyValue(newValue);
}

const EnumMap *ValueProperty::GetEnumMap() const
{
    return NULL;
}

void ValueProperty::ResetValue()
{
    overridden = false;
    ApplyValue(defaultValue);
}

bool ValueProperty::IsOverridden() const
{
    bool overriddenLocally = IsOverriddenLocally();
    if (overriddenLocally || prototypeProperty == nullptr)
        return overriddenLocally;
    
    return prototypeProperty->IsOverridden();
}

bool ValueProperty::IsOverriddenLocally() const
{
    return overridden;
}

VariantType ValueProperty::GetSubValue(int index) const
{
    return GetValueComponent(GetValue(), index);
}

void ValueProperty::SetSubValue(int index, const DAVA::VariantType &newValue)
{
    SetValue(ChangeValueComponent(GetValue(), newValue, index));
}

VariantType ValueProperty::GetDefaultSubValue(int index) const
{
    return GetValueComponent(defaultValue, index);
}

void ValueProperty::SetDefaultSubValue(int index, const DAVA::VariantType &newValue)
{
    SetDefaultValue(ChangeValueComponent(defaultValue, newValue, index));
}

int32 ValueProperty::GetStylePropertyIndex() const
{
    return stylePropertyIndex;
}

void ValueProperty::ApplyValue(const DAVA::VariantType &value)
{
}

void ValueProperty::SetName(const DAVA::String &newName)
{
    name = newName;
}

void ValueProperty::SetOverridden(bool anOverridden)
{
    overridden = anOverridden;
}

void ValueProperty::SetStylePropertyIndex(int32 index)
{
    stylePropertyIndex = index;
}

void ValueProperty::AddSubValueProperty(SubValueProperty *prop)
{
    children.push_back(SafeRetain(prop));
}

VariantType ValueProperty::ChangeValueComponent(const VariantType &value, const VariantType &component, int32 index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
        {
            Vector2 val = value.AsVector2();
            if (index == 0)
                val.x = component.AsFloat();
            else
                val.y = component.AsFloat();
            
            return VariantType(val);
        }
            
        case VariantType::TYPE_COLOR:
        {
            Color val = value.AsColor();
            if (0 <= index && index < 4)
            {
                val.color[index ] = component.AsFloat();
            }
            else
            {
                DVASSERT(false);
            }
            
            return VariantType(val);
        }
            
        case VariantType::TYPE_VECTOR4:
        {
            Vector4 val = value.AsVector4();
            if (0 <= index && index < 4)
            {
                val.data[index] = component.AsFloat();
            }
            else
            {
                DVASSERT(false);
            }
            return VariantType(val);
        }
            
        case VariantType::TYPE_INT32:
            if (GetType() == TYPE_FLAGS)
            {
                const EnumMap *map = GetEnumMap();
                int32 intValue = value.AsInt32();
                
                int val = 0;
                map->GetValue(index, val);
                if (component.AsBool())
                    return VariantType(intValue | val);
                else
                    return VariantType(intValue & (~val));
            }
            else
            {
                DVASSERT(false);
            }
            break;
            
        default:
            DVASSERT(false);
            break;
    }
    return VariantType();
}

DAVA::VariantType ValueProperty::GetValueComponent(const DAVA::VariantType &value, DAVA::int32 index) const
{
    switch (defaultValue.GetType())
    {
        case VariantType::TYPE_VECTOR2:
        {
            DVASSERT(index >= 0 && index < 2);
            return VariantType(value.AsVector2().data[index]);
        }
            
        case VariantType::TYPE_COLOR:
        {
            DVASSERT(index >= 0 && index < 4);
            return VariantType(value.AsColor().color[index]);
        }
            
        case VariantType::TYPE_VECTOR4:
        {
            DVASSERT(index >= 0 && index < 4);
            return VariantType(value.AsVector4().data[index]);
        }
            
        case VariantType::TYPE_INT32:
            if (GetType() == TYPE_FLAGS)
            {
                const EnumMap *map = GetEnumMap();
                int val = 0;
                map->GetValue(index, val);
                return VariantType((value.AsInt32() & val) != 0);
            }
            else
            {
                DVASSERT(false);
                return VariantType();
            }
            
        default:
            DVASSERT(false);
            return VariantType();
    }
}
