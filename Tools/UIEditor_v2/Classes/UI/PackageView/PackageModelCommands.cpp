#include "PackageModelCommands.h"

#include "UIPackageModel.h"
#include "UIControls/PackageHierarchy/ControlNode.h"

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

InsertControlNodeCommand::InsertControlNodeCommand(UIPackageModel *_package, const QString &controlName, int dstRow, const QModelIndex &dstParent, QUndoCommand *parent)
    : BasePackageModelCommand(_package, "Insert Control", parent)
    , dstRow(dstRow)
    , dstParent(dstParent)
    , controlName(controlName)
{
    
}

InsertControlNodeCommand::~InsertControlNodeCommand()
{
    
}

void InsertControlNodeCommand::undo()
{
    QModelIndex dstIndex = GetModel()->index(dstRow, 0, dstParent);
    GetModel()->RemoveItem(dstIndex);
}

void InsertControlNodeCommand::redo()
{
    GetModel()->InsertItem(controlName, dstRow, dstParent);
}

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
