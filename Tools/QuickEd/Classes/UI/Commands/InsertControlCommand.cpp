#include "InsertControlCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "UI/Package/PackageModel.h"

using namespace DAVA;

InsertControlCommand::InsertControlCommand(PackageModel *_model, ControlNode *_node, ControlsContainerNode *_dest, int _index, QUndoCommand *parent)
    : QUndoCommand(parent)
    , model(_model)
    , node(SafeRetain(_node))
    , dest(SafeRetain(_dest))
    , index(_index)
{
    
}

InsertControlCommand::~InsertControlCommand()
{
    model = nullptr;
    SafeRelease(node);
    SafeRelease(dest);
}

void InsertControlCommand::undo()
{
    model->RemoveControlNode(node, dest);
}

void InsertControlCommand::redo()
{
    model->InsertControlNode(node, dest, index);
}
