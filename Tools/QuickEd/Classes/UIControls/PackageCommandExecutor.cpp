#include "PackageCommandExecutor.h"

#include "PackageHierarchy/ControlNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"

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
    
void DefaultPackageCommandExecutor::InsertControlIntoPackage(ControlNode *control, PackageControlsNode *package)
{
    package->Add(control);
}

void DefaultPackageCommandExecutor::InsertControlIntoParentControl(ControlNode *control, ControlNode *parentControl)
{
    parentControl->Add(control);
}

void DefaultPackageCommandExecutor::AddImportedPackageIntoPackage(PackageControlsNode *importedPackageControls, PackageNode *package)
{
    package->GetImportedPackagesNode()->Add(importedPackageControls);
}
