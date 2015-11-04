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


#include "SubValueProperty.h"

#include "ValueProperty.h"

using namespace DAVA;

SubValueProperty::SubValueProperty(int anIndex, const DAVA::String &propName)
    : index(anIndex)
    , name(propName)
{
}

SubValueProperty::~SubValueProperty()
{
}

uint32 SubValueProperty::GetCount() const
{
    return 0;
}

AbstractProperty *SubValueProperty::GetProperty(int index) const
{
    return NULL;
}

void SubValueProperty::Accept(PropertyVisitor *visitor)
{
    DVASSERT(false);
}

const DAVA::String &SubValueProperty::GetName() const
{
    return name;
}

SubValueProperty::ePropertyType SubValueProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType SubValueProperty::GetValue() const
{
    return GetValueProperty()->GetSubValue(index);
}

void SubValueProperty::SetValue(const DAVA::VariantType &newValue)
{
    GetValueProperty()->SetSubValue(index, newValue);
}

VariantType SubValueProperty::GetDefaultValue() const
{
    return GetValueProperty()->GetDefaultSubValue(index);
}

void SubValueProperty::SetDefaultValue(const DAVA::VariantType &newValue)
{
    GetValueProperty()->SetDefaultSubValue(index, newValue);
}

void SubValueProperty::ResetValue()
{
    GetValueProperty()->ResetValue();
}

bool SubValueProperty::IsOverriddenLocally() const
{
    return GetValueProperty()->IsOverriddenLocally();
}

ValueProperty *SubValueProperty::GetValueProperty() const
{
    return DynamicTypeCheck<ValueProperty*>(GetParent());
}
