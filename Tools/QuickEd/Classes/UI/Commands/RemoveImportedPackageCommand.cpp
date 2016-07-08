#include "RemoveImportedPackageCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ImportedPackagesNode.h"

using namespace DAVA;

RemoveImportedPackageCommand::RemoveImportedPackageCommand(PackageNode* aRoot, PackageNode* anImportedPackage, QUndoCommand* parent)
    : QUndoCommand(parent)
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

void RemoveImportedPackageCommand::redo()
{
    root->RemoveImportedPackage(importedPackage);
}

void RemoveImportedPackageCommand::undo()
{
    root->InsertImportedPackage(importedPackage, index);
}
