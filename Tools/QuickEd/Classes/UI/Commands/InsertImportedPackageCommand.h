#ifndef __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
#define __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__

#include <QUndoCommand>

class PackageModel;
class PackageNode;
class PackageControlsNode;

class InsertImportedPackageCommand : public QUndoCommand
{
public:
    InsertImportedPackageCommand(PackageModel *_model, PackageControlsNode *importedPackageControls, PackageNode *dest, int index, QUndoCommand *parent = nullptr);
    virtual ~InsertImportedPackageCommand();
    
    void undo() override;
    void redo() override;
    
private:
    PackageModel *model;
    PackageControlsNode *importedPackageControls;
    PackageNode *dest;
    int index;
};

#endif // __QUICKED_INSERT_IMPORTED_PACKAGE_COMMAND_H__
