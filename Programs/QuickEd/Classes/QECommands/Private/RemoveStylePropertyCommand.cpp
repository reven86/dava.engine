#include "QECommands/RemoveStylePropertyCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

RemoveStylePropertyCommand::RemoveStylePropertyCommand(PackageNode* package, StyleSheetNode* node_, StyleSheetProperty* property_)
    : QEPackageCommand(package, ADD_REMOVE_STYLE_PROPERTY_COMMAND, "AddRemoveStyleProperty")
    , node(SafeRetain(node_))
    , property(SafeRetain(property_))
{
}

RemoveStylePropertyCommand::~RemoveStylePropertyCommand()
{
    SafeRelease(node);
    SafeRelease(property);
}

void RemoveStylePropertyCommand::Redo()
{
    package->RemoveStyleProperty(node, property);
}

void RemoveStylePropertyCommand::Undo()
{
    package->AddStyleProperty(node, property);
}
