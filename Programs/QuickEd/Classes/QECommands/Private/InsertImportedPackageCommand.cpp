#include "QECommands/InsertImportedPackageCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

InsertImportedPackageCommand::InsertImportedPackageCommand(PackageNode* package, PackageNode* importedPackage_, int index_)
    : QEPackageCommand(package, INSERT_IMPORTED_PACKAGE_COMMAND, "InsertImportedPackage")
    , importedPackage(SafeRetain(importedPackage_))
    , index(index_)
{
}

InsertImportedPackageCommand::~InsertImportedPackageCommand()
{
    SafeRelease(importedPackage);
}

void InsertImportedPackageCommand::Redo()
{
    package->InsertImportedPackage(importedPackage, index);
}

void InsertImportedPackageCommand::Undo()
{
    package->RemoveImportedPackage(importedPackage);
}
