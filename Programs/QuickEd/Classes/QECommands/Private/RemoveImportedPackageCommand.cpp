#include "QECommands/RemoveImportedPackageCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"

using namespace DAVA;

RemoveImportedPackageCommand::RemoveImportedPackageCommand(PackageNode* package, PackageNode* importedPackage_)
    : QEPackageCommand(package, REMOVE_IMPORTED_PACKAGE_COMMAND, "RemoveImportedPackage")
    , importedPackage(SafeRetain(importedPackage_))
    , index(0)
{
    index = package->GetImportedPackagesNode()->GetIndex(importedPackage);
}

RemoveImportedPackageCommand::~RemoveImportedPackageCommand()
{
    SafeRelease(importedPackage);
}

void RemoveImportedPackageCommand::Redo()
{
    package->RemoveImportedPackage(importedPackage);
}

void RemoveImportedPackageCommand::Undo()
{
    package->InsertImportedPackage(importedPackage, index);
}
