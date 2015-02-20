#include "RemoveControlCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "UI/Package/PackageModel.h"

using namespace DAVA;

RemoveControlCommand::RemoveControlCommand(PackageModel *_model, ControlNode *_node, ControlsContainerNode *_dest, int _index, QUndoCommand *parent)
    : QUndoCommand(parent)
    , model(_model)
    , node(SafeRetain(_node))
    , dest(SafeRetain(_dest))
    , index(_index)
{
    
}

RemoveControlCommand::~RemoveControlCommand()
{
    model = nullptr;
    SafeRelease(node);
    SafeRelease(dest);
}

void RemoveControlCommand::undo()
{
    node->MarkAsAlive();
    model->InsertControlNode(node, dest, index);
}

void RemoveControlCommand::redo()
{
    node->MarkAsRemoved();
    model->RemoveControlNode(node, dest);
}
