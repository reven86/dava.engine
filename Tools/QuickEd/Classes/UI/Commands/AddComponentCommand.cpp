#include "AddComponentCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/Properties/PropertiesModel.h"

using namespace DAVA;

AddComponentCommand::AddComponentCommand(PropertiesModel *_model, ControlNode *_node, int _componentType, QUndoCommand *parent)
    : QUndoCommand(parent)
    , model(_model)
    , node(SafeRetain(_node))
    , componentType(_componentType)
{
    
}

AddComponentCommand::~AddComponentCommand()
{
    SafeRelease(node);
}

void AddComponentCommand::redo()
{
    model->AddComponent(node, componentType);
}

void AddComponentCommand::undo()
{
    
}
