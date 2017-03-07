#include "QECommands/AddStylePropertyCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddStylePropertyCommand::AddStylePropertyCommand(PackageNode* package, StyleSheetNode* node_, StyleSheetProperty* property_)
    : QEPackageCommand(package, ADD_REMOVE_STYLE_PROPERTY_COMMAND, "AddRemoveStyleProperty")
    , node(SafeRetain(node_))
    , property(SafeRetain(property_))
{
}

AddStylePropertyCommand::~AddStylePropertyCommand()
{
    SafeRelease(node);
    SafeRelease(property);
}

void AddStylePropertyCommand::Redo()
{
    package->AddStyleProperty(node, property);
}

void AddStylePropertyCommand::Undo()
{
    package->RemoveStyleProperty(node, property);
}
