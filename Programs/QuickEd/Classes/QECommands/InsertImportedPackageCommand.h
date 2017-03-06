#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class PackageControlsNode;

class InsertImportedPackageCommand : public QEPackageCommand
{
public:
    InsertImportedPackageCommand(PackageNode* package, PackageNode* importedPackage, int index);
    ~InsertImportedPackageCommand() override;

    void Redo() override;
    void Undo() override;

private:
    PackageNode* importedPackage = nullptr;
    const int index;
};

