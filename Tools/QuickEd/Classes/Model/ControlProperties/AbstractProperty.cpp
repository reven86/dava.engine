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


#include "AbstractProperty.h"

using namespace DAVA;


AbstractProperty::AbstractProperty() : parent(NULL)
{
}

AbstractProperty::~AbstractProperty()
{
}

AbstractProperty *AbstractProperty::GetParent() const
{
    return parent;
}

void AbstractProperty::SetParent(AbstractProperty *parent)
{
    this->parent = parent;
}

int AbstractProperty::GetIndex(AbstractProperty *property) const
{
    for (int32 i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i) == property)
            return i;
    }
    return -1;
}

void AbstractProperty::Refresh(DAVA::int32 refreshFlags)
{
}

AbstractProperty *AbstractProperty::FindPropertyByPrototype(AbstractProperty *prototype)
{
    for (int32 i = 0; i < GetCount(); i++)
    {
        AbstractProperty *result = GetProperty(i)->FindPropertyByPrototype(prototype);
        if (result)
            return result;
    }
    return nullptr;
}

bool AbstractProperty::HasChanges() const
{
    for (int32 i = 0; i < GetCount(); i++)
    {
        if (GetProperty(i)->HasChanges())
            return true;
    }
    return false;
}

uint32 AbstractProperty::GetFlags() const
{
    return EF_NONE;
}

int32 AbstractProperty::GetStylePropertyIndex() const
{
    return -1;
}

bool AbstractProperty::IsReadOnly() const
{
    return parent ? parent->IsReadOnly() : true;
}

DAVA::VariantType AbstractProperty::GetValue() const
{
    return DAVA::VariantType();
}

void AbstractProperty::SetValue(const DAVA::VariantType &/*newValue*/)
{
    // Do nothing by default
}

VariantType AbstractProperty::GetDefaultValue() const
{
    return VariantType();
}

void AbstractProperty::SetDefaultValue(const DAVA::VariantType &newValue)
{
    // Do nothing by default
}

const EnumMap *AbstractProperty::GetEnumMap() const
{
    return NULL;
}

void AbstractProperty::ResetValue()
{
    // Do nothing by default
}

bool AbstractProperty::IsOverridden() const
{
    return false; // false by default
}

bool AbstractProperty::IsOverriddenLocally() const
{
    return false; // false by default
}

AbstractProperty *AbstractProperty::GetRootProperty()
{
    AbstractProperty *property = this;
    while (property->parent)
        property = property->parent;
    return property;
}

const AbstractProperty *AbstractProperty::GetRootProperty() const
{
    const AbstractProperty *property = this;
    while (property->parent)
        property = property->parent;
    return property;
}
