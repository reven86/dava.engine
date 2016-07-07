#include "RemoveImportedPackageCommand.h"

#include "Document/CommandsBase/QECommandIDs.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"

using namespace DAVA;

RemoveImportedPackageCommand::RemoveImportedPackageCommand(PackageNode* aRoot, PackageNode* anImportedPackage)
    : QECommand(CMDID_REMOVE_IMPORTED_PACKAGE, "RemoveImportedPackage")
    , root(SafeRetain(aRoot))
    , importedPackage(SafeRetain(anImportedPackage))
    , index(0)
{
    index = root->GetImportedPackagesNode()->GetIndex(importedPackage);
}

RemoveImportedPackageCommand::~RemoveImportedPackageCommand()
{
    SafeRelease(importedPackage);
    SafeRelease(root);
}

void RemoveImportedPackageCommand::Redo()
{
    root->RemoveImportedPackage(importedPackage);
}

void RemoveImportedPackageCommand::Undo()
{
    root->InsertImportedPackage(importedPackage, index);
}
