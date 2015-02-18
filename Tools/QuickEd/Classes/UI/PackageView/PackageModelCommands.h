#ifndef __UI_EDITOR_PACKAGE_MODEL_COMMANDS_H__
#define __UI_EDITOR_PACKAGE_MODEL_COMMANDS_H__

#include <QUndoCommand>
#include <QModelIndex>

class UIPackageModel;
class ControlNode;
class PackageControlsNode;

class BasePackageModelCommand: public QUndoCommand
{
public:
    BasePackageModelCommand(UIPackageModel *_packageModel, const QString &text, QUndoCommand * parent = 0)
    : QUndoCommand(text, parent)
    , packageModel(_packageModel)
    {}
    
    virtual ~BasePackageModelCommand()
    {
    }
    
    UIPackageModel *GetModel() {
        return packageModel;
    }
    
private:
    UIPackageModel *packageModel;
};

class MoveItemModelCommand: public BasePackageModelCommand
{
public:
    MoveItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    ~MoveItemModelCommand();
    
    virtual void undo();
    virtual void redo();
private:
    int srcRow;
    int dstRow;
    QPersistentModelIndex srcParent;
    QPersistentModelIndex dstParent;
};

class CopyItemModelCommand: public BasePackageModelCommand
{
public:
    CopyItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    ~CopyItemModelCommand();
    
    virtual void undo();
    virtual void redo();
private:
    int srcRow;
    int dstRow;
    QPersistentModelIndex srcParent;
    QPersistentModelIndex dstParent;
};

class InsertControlNodeCommand: public BasePackageModelCommand
{
public:
    InsertControlNodeCommand(UIPackageModel *_package, const QString &controlName, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    InsertControlNodeCommand(UIPackageModel *_package, ControlNode *control, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    ~InsertControlNodeCommand();
    
    virtual void undo();
    virtual void redo();
private:
    int dstRow;
    QPersistentModelIndex dstParent;
    QString controlName;
    ControlNode *control;
};

class InsertImportedPackageCommand: public BasePackageModelCommand
{
public:
    InsertImportedPackageCommand(UIPackageModel *_package, PackageControlsNode *_importedPackage, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    ~InsertImportedPackageCommand();
    
    virtual void undo();
    virtual void redo();

private:
    int dstRow;
    QPersistentModelIndex dstParent;
    PackageControlsNode *importedPackage;
};

class RemoveControlNodeCommand: public BasePackageModelCommand
{
public:
    RemoveControlNodeCommand(UIPackageModel *_package, int row, const QModelIndex &parentIndex, QUndoCommand *parent = 0);
    ~RemoveControlNodeCommand();
    
    virtual void undo();
    virtual void redo();
private:
    QPersistentModelIndex parentIndex;
    int row;
    ControlNode *node;
    
};
#endif // __UI_EDITOR_PACKAGE_MODEL_COMMANDS_H__
