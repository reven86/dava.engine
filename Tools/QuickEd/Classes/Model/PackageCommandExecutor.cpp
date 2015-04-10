#include "PackageCommandExecutor.h"

#include "PackageHierarchy/ControlNode.h"
#include "PackageHierarchy/ControlsContainerNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"

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

void DefaultPackageCommandExecutor::ChangeProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &value)
{
    property->SetValue(value);
}

void DefaultPackageCommandExecutor::ResetProperty(ControlNode *node, BaseProperty *property)
{
    property->ResetValue();
}

void DefaultPackageCommandExecutor::AddComponent(ControlNode *node, DAVA::uint32 componentType)
{
    node->GetPropertiesRoot()->AddComponentPropertiesSection(componentType);
}

void DefaultPackageCommandExecutor::RemoveComponent(ControlNode *node, DAVA::uint32 componentType)
{
    node->GetPropertiesRoot()->RemoveComponentPropertiesSection(componentType);
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
