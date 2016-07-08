#ifndef __QUICKED_REMOVE_IMPORTED_PACKAGE_COMMAND_H__
#define __QUICKED_REMOVE_IMPORTED_PACKAGE_COMMAND_H__

#include <QUndoCommand>

class PackageNode;
class PackageControlsNode;

class RemoveImportedPackageCommand : public QUndoCommand
{
public:
    RemoveImportedPackageCommand(PackageNode* aRoot, PackageNode* anImportedPackage, QUndoCommand* parent = nullptr);
    virtual ~RemoveImportedPackageCommand();

    void redo() override;
    void undo() override;

private:
    PackageNode* root;
    PackageNode* importedPackage;
    int index;
};

#endif // __QUICKED_REMOVE_IMPORTED_PACKAGE_COMMAND_H__
