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


#include "NameProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"

#include "UI/UIControl.h"

using namespace DAVA;

NameProperty::NameProperty(ControlNode *anControl, const NameProperty *sourceProperty, eCloneType cloneType)
    : ValueProperty("Name")
    , control(anControl) // weak ptr
{
    if (sourceProperty)
    {
        control->GetControl()->SetName(sourceProperty->GetValue().AsString());

        if (cloneType == CT_INHERIT && control->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
        {
            AttachPrototypeProperty(sourceProperty);
        }
    }
}

NameProperty::~NameProperty()
{
    control = nullptr; // weak ptr
}

void NameProperty::Refresh(DAVA::int32 refreshFlags)
{
    ValueProperty::Refresh(refreshFlags);
    
    if ((refreshFlags & REFRESH_DEFAULT_VALUE) != 0 && GetPrototypeProperty())
        ApplyValue(GetDefaultValue());
}

void NameProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitNameProperty(this);
}

bool NameProperty::IsReadOnly() const
{
    return control->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD || ValueProperty::IsReadOnly();
}

NameProperty::ePropertyType NameProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::uint32 NameProperty::GetFlags() const
{
    return EF_AFFECTS_STYLES;
}

VariantType NameProperty::GetValue() const
{
    return VariantType(control->GetName());
}

bool NameProperty::IsOverriddenLocally() const
{
    return control->GetCreationType() != ControlNode::CREATED_FROM_PROTOTYPE_CHILD;
}

ControlNode *NameProperty::GetControlNode() const
{
    return control;
}

void NameProperty::ApplyValue(const DAVA::VariantType &value)
{
    if (value.GetType() == VariantType::TYPE_STRING)
    {
        control->GetControl()->SetName(value.AsString());
    }
    else
    {
        DVASSERT(false);
    }
}
