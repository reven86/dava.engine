#include "MoveControlCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/ControlsContainerNode.h"
#include "UI/Package/PackageModel.h"

using namespace DAVA;

MoveControlCommand::MoveControlCommand(PackageModel *_model, ControlNode *_node, ControlsContainerNode *_src, int _srcIndex, ControlsContainerNode *_dest, int _destIndex, QUndoCommand *parent)
    : QUndoCommand(parent)
    , model(_model)
    , node(SafeRetain(_node))
    , src(SafeRetain(_src))
    , srcIndex(_srcIndex)
    , dest(SafeRetain(_dest))
    , destIndex(_destIndex)
{
    
}

MoveControlCommand::~MoveControlCommand()
{
    model = nullptr;
    SafeRelease(node);
    SafeRelease(src);
    SafeRelease(dest);
}

void MoveControlCommand::undo()
{
    model->MoveControlNode(node, dest, destIndex, src, srcIndex);
}

void MoveControlCommand::redo()
{
    model->MoveControlNode(node, src, srcIndex, dest, destIndex);
}
