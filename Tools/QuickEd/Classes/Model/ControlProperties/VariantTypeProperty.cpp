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

#include "VariantTypeProperty.h"

#include "PropertyVisitor.h"
#include "IntrospectionProperty.h"

#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

VariantTypeProperty::VariantTypeProperty(const String &name, VariantType &vt)
    : ValueProperty(name)
    , value(vt)
{
    SetOverridden(true);
}

VariantTypeProperty::~VariantTypeProperty()
{
}

void VariantTypeProperty::Accept(PropertyVisitor *visitor)
{
    // do nothing
}

bool VariantTypeProperty::IsReadOnly() const
{
    return GetParent() == nullptr ? true : GetParent()->IsReadOnly();
}

AbstractProperty::ePropertyType VariantTypeProperty::GetType() const
{
    return TYPE_VARIANT;
}

VariantType VariantTypeProperty::GetValue() const
{
    return value;
}

const EnumMap *VariantTypeProperty::GetEnumMap() const
{
    return nullptr;
}

void VariantTypeProperty::ApplyValue(const DAVA::VariantType &newValue)
{
    value = newValue;
}

