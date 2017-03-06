#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class PackageNode;
class PackageControlsNode;

class RemoveImportedPackageCommand : public QEPackageCommand
{
public:
    RemoveImportedPackageCommand(PackageNode* package, PackageNode* importedPackage);
    ~RemoveImportedPackageCommand() override;

    void Redo() override;
    void Undo() override;

private:
    PackageNode* importedPackage = nullptr;
    int index = -1;
};
