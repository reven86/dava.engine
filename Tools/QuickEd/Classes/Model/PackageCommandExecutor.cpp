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


#include "PackageCommandExecutor.h"

#include "PackageHierarchy/ControlNode.h"
#include "PackageHierarchy/ControlsContainerNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "ControlProperties/AbstractProperty.h"
#include "ControlProperties/RootProperty.h"

using namespace DAVA;

PackageCommandExecutor::PackageCommandExecutor()
{
    
}

PackageCommandExecutor::~PackageCommandExecutor()
{
    
}

////////////////////////////////////////////////////////////////////////////////

DefaultPackageCommandExecutor::DefaultPackageCommandExecutor()
{
    
}

DefaultPackageCommandExecutor::~DefaultPackageCommandExecutor()
{
    
}
    
void DefaultPackageCommandExecutor::AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package)
{
    package->GetImportedPackagesNode()->Add(importedPackageControls);
}

void DefaultPackageCommandExecutor::ChangeProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &value)
{
    property->SetValue(value);
}

void DefaultPackageCommandExecutor::ResetProperty(ControlNode *node, AbstractProperty *property)
{
    property->ResetValue();
}

void DefaultPackageCommandExecutor::AddComponent(ControlNode *node, DAVA::uint32 componentType)
{
    node->GetRootProperty()->AddComponentPropertiesSection(componentType);
}

void DefaultPackageCommandExecutor::RemoveComponent(ControlNode *node, DAVA::uint32 componentType, DAVA::uint32 componentIndex)
{
    node->GetRootProperty()->RemoveComponentPropertiesSection(componentType, componentIndex);
}

void DefaultPackageCommandExecutor::InsertControl(ControlNode *control, ControlsContainerNode *dest, int32 destIndex)
{
    dest->InsertAtIndex(destIndex, control);
}

void DefaultPackageCommandExecutor::CopyControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, int32 destIndex)
{
    DVASSERT_MSG(false, "Implement me"); // TODO implement
}

void DefaultPackageCommandExecutor::MoveControls(const DAVA::Vector<ControlNode*> &nodes, ControlsContainerNode *dest, int32 destIndex)
{
    DVASSERT_MSG(false, "Implement me"); // TODO implement
}

void DefaultPackageCommandExecutor::RemoveControls(const DAVA::Vector<ControlNode*> &nodes)
{
    DVASSERT_MSG(false, "Implement me"); // TODO implement
}
