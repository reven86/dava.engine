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


#include "PrototypeNameProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"
#include "../PackageHierarchy/PackageNode.h"

using namespace DAVA;

PrototypeNameProperty::PrototypeNameProperty(ControlNode *aNode, const PrototypeNameProperty *sourceProperty, eCloneType cloneType)
    : ValueProperty("Prototype")
    , node(aNode) // weak
{
}

PrototypeNameProperty::~PrototypeNameProperty()
{
    node = nullptr; // weak
}

void PrototypeNameProperty::Accept(PropertyVisitor *visitor)
{
    visitor->VisitPrototypeNameProperty(this);
}

AbstractProperty::ePropertyType PrototypeNameProperty::GetType() const
{
    return TYPE_VARIANT;
}

DAVA::VariantType PrototypeNameProperty::GetValue() const
{
    return VariantType(GetPrototypeName());
}

bool PrototypeNameProperty::IsReadOnly() const
{
    return true;
}

String PrototypeNameProperty::GetPrototypeName() const
{
    ControlNode *prototype = node->GetPrototype();
    if (prototype)
    {
        String path = "";
        if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
        {
            path = String("/") + node->GetPathToPrototypeChild();
        }
        
        
        const PackageNode *package = prototype->GetPackage();
        if (package && package->IsImported())
        {
            return package->GetName() + "/" + prototype->GetName() + path;
        }
        else
        {
            return prototype->GetName() + path;
        }
    }
    
    return String("");
}

ControlNode *PrototypeNameProperty::GetControl() const
{
    return node;
}

void PrototypeNameProperty::ApplyValue(const DAVA::VariantType &value)
{
    // do nothing
}
