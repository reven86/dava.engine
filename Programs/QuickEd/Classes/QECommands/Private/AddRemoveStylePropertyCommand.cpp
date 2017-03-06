#include "QECommands/AddRemoveStylePropertyCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddRemoveStylePropertyCommand::AddRemoveStylePropertyCommand(PackageNode* package, StyleSheetNode* node_, StyleSheetProperty* property_, bool add_)
    : QEPackageCommand(package, ADD_REMOVE_STYLE_PROPERTY_COMMAND, "AddRemoveStyleProperty")
    , node(SafeRetain(node_))
    , property(SafeRetain(property_))
    , add(add_)
{
}

AddRemoveStylePropertyCommand::~AddRemoveStylePropertyCommand()
{
    SafeRelease(node);
    SafeRelease(property);
}

void AddRemoveStylePropertyCommand::Redo()
{
    if (add)
        package->AddStyleProperty(node, property);
    else
        package->RemoveStyleProperty(node, property);
}

void AddRemoveStylePropertyCommand::Undo()
{
    if (add)
        package->RemoveStyleProperty(node, property);
    else
        package->AddStyleProperty(node, property);
}
