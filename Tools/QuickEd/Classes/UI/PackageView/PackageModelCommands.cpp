#include "PackageModelCommands.h"

#include "UIPackageModel.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageControlsNode.h"

////////////////////////////////////////////////////////////////////////////////
// MoveItemModelCommand
////////////////////////////////////////////////////////////////////////////////

MoveItemModelCommand::MoveItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int _dstRow, const QModelIndex &_dstParent, QUndoCommand *parent)
    : BasePackageModelCommand(_package, "Move item", parent)
    , srcRow(srcIndex.row())
    , dstRow(_dstRow)
    , srcParent(srcIndex.parent())
    , dstParent(_dstParent)
{
    
}

MoveItemModelCommand::~MoveItemModelCommand()
{
    
}

void MoveItemModelCommand::undo()
{
    int fixedDstRow = dstRow;
    int fixedSrcRow = srcRow;
    if (srcParent == dstParent)
    {
        if (dstRow > srcRow)
        {
            --fixedDstRow;
        }
        else
        {
            ++fixedSrcRow;
        }
        
    }
    QModelIndex dstIndex = GetModel()->index(fixedDstRow, 0, dstParent);
    GetModel()->MoveItem(dstIndex, fixedSrcRow, srcParent);
}

void MoveItemModelCommand::redo()
{
    QModelIndex srcIndex = GetModel()->index(srcRow, 0, srcParent);
    GetModel()->MoveItem(srcIndex, dstRow, dstParent);
}

////////////////////////////////////////////////////////////////////////////////
// CopyItemModelCommand
////////////////////////////////////////////////////////////////////////////////

CopyItemModelCommand::CopyItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int _dstRow, const QModelIndex &_dstParent, QUndoCommand *parent)
    : BasePackageModelCommand(_package, "Move item", parent)
    , srcRow(srcIndex.row())
    , dstRow(_dstRow)
    , srcParent(srcIndex.parent())
    , dstParent(_dstParent)
{
    
}

CopyItemModelCommand::~CopyItemModelCommand()
{
    
}

void CopyItemModelCommand::undo()
{
    QModelIndex dstIndex = GetModel()->index(dstRow, 0, dstParent);
    GetModel()->RemoveItem(dstIndex);
}

void CopyItemModelCommand::redo()
{
    QModelIndex srcIndex = GetModel()->index(srcRow, 0, srcParent);
    GetModel()->CopyItem(srcIndex, dstRow, dstParent);
}


////////////////////////////////////////////////////////////////////////////////
// InsertControlNodeCommand
////////////////////////////////////////////////////////////////////////////////

InsertControlNodeCommand::InsertControlNodeCommand(UIPackageModel *_package, const QString &_controlName, int dstRow, const QModelIndex &dstParent, QUndoCommand *parent)
    : InsertControlNodeCommand(_package, nullptr, dstRow, dstParent, parent)
{
    controlName = _controlName;
}

InsertControlNodeCommand::InsertControlNodeCommand(UIPackageModel *_package, ControlNode *_control, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent)
    : BasePackageModelCommand(_package, "Insert Control", parent)
    , dstRow(dstRow)
    , dstParent(dstParent)
    , controlName("")
    , control(SafeRetain(_control))
{
    
}

InsertControlNodeCommand::~InsertControlNodeCommand()
{
    SafeRelease(control);
}

void InsertControlNodeCommand::undo()
{
    QModelIndex dstIndex = GetModel()->index(dstRow, 0, dstParent);
    GetModel()->RemoveItem(dstIndex);
}

void InsertControlNodeCommand::redo()
{
    if (control)
        GetModel()->InsertItem(control, dstRow, dstParent);
    else
        GetModel()->InsertItem(controlName, dstRow, dstParent);
}


////////////////////////////////////////////////////////////////////////////////
// RemoveControlNodeCommand
////////////////////////////////////////////////////////////////////////////////

InsertImportedPackageCommand::InsertImportedPackageCommand(UIPackageModel *_package, PackageControlsNode *_importedPackage, int _dstRow, const QModelIndex &_dstParent, QUndoCommand * parent)
    : BasePackageModelCommand(_package, "Insert Package", parent)
    , dstRow(_dstRow)
    , dstParent(_dstParent)
    , importedPackage(SafeRetain(_importedPackage))
{
    
}

InsertImportedPackageCommand::~InsertImportedPackageCommand()
{
    SafeRelease(importedPackage);
}

void InsertImportedPackageCommand::undo()
{
    QModelIndex dstIndex = GetModel()->index(dstRow, 0, dstParent);
    GetModel()->RemoveItem(dstIndex);
}

void InsertImportedPackageCommand::redo()
{
    GetModel()->InsertImportedPackage(importedPackage, dstRow, dstParent);
}

//int dstRow;
//QPersistentModelIndex dstParent;
//PackageControlsNode *importedPackage;

////////////////////////////////////////////////////////////////////////////////
// RemoveControlNodeCommand
////////////////////////////////////////////////////////////////////////////////

RemoveControlNodeCommand::RemoveControlNodeCommand(UIPackageModel *_package, int row, const QModelIndex &parentIndex, QUndoCommand *parent)
    : BasePackageModelCommand(_package, "Remove Control", parent)
    , parentIndex(parentIndex)
    , row(row)
    , node(NULL)
{
    QModelIndex index = parentIndex.child(row, 0);
    node = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(index.internalPointer()));
    SafeRetain(node);
    DVASSERT(node);
}

RemoveControlNodeCommand::~RemoveControlNodeCommand()
{
    SafeRelease(node);
}

void RemoveControlNodeCommand::undo()
{
    GetModel()->InsertItem(node, row, parentIndex);
}

void RemoveControlNodeCommand::redo()
{
    GetModel()->RemoveItem(parentIndex.child(row, 0));
}
