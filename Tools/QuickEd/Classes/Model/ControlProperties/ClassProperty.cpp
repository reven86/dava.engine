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


#include "ClassProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"

#include "UI/UIControl.h"

using namespace DAVA;

ClassProperty::ClassProperty(ControlNode *aControl)
    : ValueProperty("Class")
    , control(aControl) // weak
{
}

ClassProperty::~ClassProperty()
{
    control = nullptr; // weak
}

void ClassProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitClassProperty(this);
}

bool ClassProperty::IsReadOnly() const
{
    return true;
}

ClassProperty::ePropertyType ClassProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::uint32 ClassProperty::GetFlags() const
{
    return EF_AFFECTS_STYLES;
}

VariantType ClassProperty::GetValue() const
{
    return VariantType(control->GetControl()->GetClassName());
}

const String &ClassProperty::GetClassName() const
{
    return control->GetControl()->GetClassName();
}

ControlNode *ClassProperty::GetControlNode() const
{
    return control;
}
